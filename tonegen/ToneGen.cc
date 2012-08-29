#include "SoundOpenAl.hpp"

#include <algorithm>

#include <cstring>
#include <cmath>
#include <cstdio>

#include <termios.h>
#include <unistd.h>

#define TWO_PI (2.0 * acos(-1.0))
#define FREQ_BASE 8000

template <typename T, typename O>
O clamp(T val, O min, O max) {
	O ret = static_cast<O>(val);

	if (ret < min) {
		return min;
	}

	if (ret > max) {
		return max;
	}

	return ret;
}

static void make_raw(int fd, bool enable) {
	struct termios term;
	tcgetattr(fd, &term);
	if (enable) {
		term.c_lflag &= ~(ICANON | ISIG);
	}
	else {
		term.c_lflag |= (ICANON | ISIG);
	}
	tcsetattr(fd, TCSANOW, &term);
}

static unsigned char buffer[FREQ_BASE];
static double last_phase = 0;

static void tone(unsigned frequency, size_t count) {
	size_t size = std::min(count, sizeof(buffer));
	memset(buffer, 0, size);

	double phase = last_phase * frequency;

	for (size_t i = 0; i < size; i++) {
		phase += (frequency * TWO_PI) / FREQ_BASE;
		double frame = 128 * (1.0 + sin(phase));
		buffer[i] = clamp(frame, 0, 255);
	}

	phase /= frequency;
	while (phase > TWO_PI) {
		phase -= TWO_PI;
	}

	last_phase = phase;
}

int main(int argc, char **argv) {
	SoundOpenAlPlayer player(1, FREQ_BASE);
	make_raw(0, true);
	
	while (1) {
		struct timeval tv = {
			tv.tv_sec = 0,
			tv.tv_usec = 1000 * 500,
		};

		fd_set read_set;
		FD_ZERO(&read_set);
		FD_SET(0, &read_set);

		select(1, &read_set, 0, 0, &tv);

		char c;
		read(0, &c, 1);

		if (c == ']') {
			break;
		}

		int chord = c - 'a';
		double mult = pow(2.0, chord / 12.0);
		unsigned fq = static_cast<unsigned>(440 * mult);

		unsigned msec = tv.tv_sec * 1000 + tv.tv_usec / 1000;
		msec = clamp(msec, 1, 1000);

		unsigned n_samples = (FREQ_BASE * msec) / 1000;
		
		printf("frequency %d msec %d samples %d\n",
			fq, msec, n_samples);

		tone(fq, n_samples);
		player.transmitData(buffer, n_samples);
		//usleep(msec * 1000);
	}

	return 0;
}
