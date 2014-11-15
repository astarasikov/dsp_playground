#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cmath>
#include <algorithm>

#define INPUT "B1.bmp"
#define OUTPUT "out.bmp"

#define WIDTH_OFFSET size_t(0x12)
#define HEIGHT_OFFSET size_t(0x16)
#define HDR_SIZE size_t(0x8a)

#define RED(c) ((c >> 16) & 0xff)
#define GREEN(c) ((c >> 8) & 0xff)
#define BLUE(c) (c & 0xff)

struct color {
  uint32_t rgb;
  double hsv[3];
};

#define FEQ(a, b) (fabs(a - b) < 0.001)

static void to_hsv(uint32_t rgb, double *hsv) {
  using namespace std;

  double b = BLUE(rgb) / 255.0;
  double g = GREEN(rgb) / 255.0;
  double r = RED(rgb) / 255.0;

  double rgb_min = min(r, min(g, b));
  double rgb_max = max(r, max(g, b));

  double h = rgb_max;
  double s = rgb_max;
  double v = rgb_max;

  double dist = rgb_max - rgb_min;

  if (FEQ(dist, 0)) {
    h = s = v = 0;
    goto done;
  }

  s = ((rgb_max == 0) ? 0 : (dist / rgb_max));

  if (FEQ(rgb_max, r)) {
    h = (g - b) / dist + (g < b ? 6 : 0);
  } else if (FEQ(rgb_max, g)) {
    h = (b - r) / dist + 2;
  } else {
    h = (r - g) / dist + 4;
  }
  h /= 6;

done:
  hsv[0] = h;
  hsv[1] = s;
  hsv[2] = v;
}

static void sort_colors(struct color *colors, size_t width) {
  auto sort_by_hue = [](color a, color b) { return a.hsv[0] <= b.hsv[0]; };
  std::stable_sort(colors, colors + width, sort_by_hue);
}

static void process(uint8_t *in, uint8_t *out) {
  uint32_t width = ((uint32_t *)(in + WIDTH_OFFSET))[0];
  uint32_t height = ((uint32_t *)(in + HEIGHT_OFFSET))[0];
  printf("Input width=%u height=%u\n", width, height);

  struct color *colors = new color[width];

  uint8_t *rgb_data = in + HDR_SIZE + ((3 * width * height) / 2);
  for (size_t i = 0; i < width; i++) {
    uint32_t rgb = rgb_data[0] + (rgb_data[1] << 8) + (rgb_data[2] << 16);
    colors[i].rgb = rgb;
    to_hsv(rgb, colors[i].hsv);
    rgb_data += 3;
  }

  std::copy(in, in + HDR_SIZE, out);
  sort_colors(colors, width);

  for (size_t h = 0; h < height; h++) {
    for (size_t w = 0; w < width; w++) {
      color *c = colors + w;
      uint8_t *out_data = out + HDR_SIZE + (3 * width * h) + 3 * w;

      out_data[0] = BLUE(c->rgb);
      out_data[1] = GREEN(c->rgb);
      out_data[2] = RED(c->rgb);
    }
  }

  delete[] colors;
}

static void grow_output(int fd, size_t size) {
#ifdef __APPLE__
  assert(lseek(fd, size - 1, SEEK_SET) >= 0);
  assert(write(fd, "\0", 1) >= 0);
  assert(lseek(fd, 0, SEEK_SET) >= 0);
#else
  assert(fallocate(fd, 0, 0, size) >= 0);
#endif
}

int main(int argc, char **argv) {
  int ret = 0;

  uint8_t *fin_data = NULL;
  uint8_t *fout_data = NULL;
  int in_fd = -1;
  int out_fd = -1;

  struct stat st_in = {};

  if (argc < 3) {
    fprintf(stderr, "Usage: %s in.bmp out.bmp\n", argv[0]);
    goto done;
  }

  assert((in_fd = open(argv[1], O_RDONLY)) >= 0);
  assert(fstat(in_fd, &st_in) >= 0);
  assert(MAP_FAILED !=
         (fin_data = (uint8_t *)mmap(NULL, st_in.st_size, PROT_READ, MAP_SHARED,
                                     in_fd, 0)));

  assert((out_fd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0770)) >= 0);
  grow_output(out_fd, st_in.st_size);
  assert(MAP_FAILED !=
         (fout_data = (uint8_t *)mmap(NULL, st_in.st_size, PROT_WRITE,
                                      MAP_SHARED, out_fd, 0)));
  process(fin_data, fout_data);
done:
  if (fin_data) {
    munmap(fin_data, st_in.st_size);
  }

  if (fout_data) {
    munmap(fout_data, st_in.st_size);
  }

  if (in_fd >= 0) {
    close(in_fd);
  }
  if (out_fd >= 0) {
    close(out_fd);
  }
  return ret;
}
