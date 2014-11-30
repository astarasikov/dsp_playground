#include <stdio.h>
#include <stdlib.h>

enum algo_impl {
  IMPL_HOST,
  IMPL_GPU_SIMPLE,
  IMPL_GPU_TILED,
};

enum {
  TILE_SIZE = 8,
};

static void simple_mmult(float *A, float *B, float *C, size_t N) {
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      C[i * N + j] = 0;
      for (size_t k = 0; k < N; k++) {
        C[i * N + j] += A[i * N + k] * B[k * N + j];
      }
    }
  }
}

__global__ void mmult_gpu_simple(float *A, float *B, float *C, size_t N) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;

  float val = 0.0f;

  for (size_t k = 0; k < N; k++) {
    val += A[N * i + k] * B[N * k + j];
  }

  C[N * i + j] = val;
}

__global__ void mmult_gpu_tiled(float *A, float *B, float *C, size_t N) {
  __shared__ float As[TILE_SIZE][TILE_SIZE];
  __shared__ float Bs[TILE_SIZE][TILE_SIZE];

  int tx = threadIdx.x;
  int ty = threadIdx.y;

  int row = blockIdx.y * TILE_SIZE + ty;
  int col = blockIdx.x * TILE_SIZE + tx;

  float val = 0.0f;

  for (size_t k = 0; k < N / TILE_SIZE; k++) {
    As[ty][tx] = A[row * N + (k * TILE_SIZE + tx)];
    Bs[ty][tx] = B[(k * TILE_SIZE + ty) * N + col];

    __syncthreads();

    for (int m = 0; m < TILE_SIZE; m++) {
      val += As[ty][m] * Bs[m][tx];
    }
    //__syncthreads();
  }
  C[N * row + col] = val;
}

static void mrand(float *A, size_t size) {
  for (int i = 0; i < size * size; i++) {
    A[i] = (float)(rand() % 1000);
  }
}

static void mdump(float *A, size_t size, const char *fname) {
  FILE *f = NULL;
  f = fopen(fname, "wb");
  if (!f) {
    perror("fopen");
    exit(-1);
  }

  for (size_t row = 0; row < size; row++) {
    for (size_t col = 0; col < size - 1; col++) {
      fprintf(f, "%4.4f,", A[row * size + col]);
    }
    fprintf(f, "%4.4f", A[row * size + size - 1]);
    fprintf(f, "\n");
  }

  if (f) {
    fclose(f);
  }
}

int main(int argc, char **argv) {
  enum algo_impl algo = IMPL_GPU_TILED;
  unsigned matrix_size = 32;

  if (argc < 2) {
    printf("Usage: %s [tiled|simple|cpu] size\n", argv[0]);
    return -1;
  }

  if (!strcmp(argv[1], "tiled")) {
    algo = IMPL_GPU_TILED;
  } else if (!strcmp(argv[1], "simple")) {
    algo = IMPL_GPU_SIMPLE;
  } else if (!strcmp(argv[1], "cpu")) {
    algo = IMPL_HOST;
  }

  if (argc > 2) {
    sscanf(argv[1], "%u", &matrix_size);
    if (matrix_size & (matrix_size - 1)) {
      fprintf(stderr, "matrix size '%u' is not power of two\n", matrix_size);
      exit(-1);
    }
  }

  srand(time(NULL));

  size_t alloc_size = matrix_size * matrix_size * sizeof(float);

  float *host_A = (float *)malloc(alloc_size);
  float *host_B = (float *)malloc(alloc_size);
  float *host_C = (float *)malloc(alloc_size);

  float *gpu_A = NULL, *gpu_B = NULL, *gpu_C = NULL;

  cudaMalloc((void **)&gpu_A, alloc_size);
  cudaMalloc((void **)&gpu_B, alloc_size);
  cudaMalloc((void **)&gpu_C, alloc_size);

  dim3 block(TILE_SIZE, TILE_SIZE, 1);
  dim3 grid(matrix_size / block.x, matrix_size / block.y);

  if (!host_A || !host_B || !host_C || !gpu_A || !gpu_B || !gpu_C) {
    return -1;
  }

  mrand(host_A, matrix_size);
  mrand(host_B, matrix_size);
  mdump(host_A, matrix_size, "mtx_a.csv");
  mdump(host_B, matrix_size, "mtx_b.csv");

  cudaMemcpy(gpu_A, host_A, alloc_size, cudaMemcpyHostToDevice);
  cudaMemcpy(gpu_B, host_B, alloc_size, cudaMemcpyHostToDevice);

  switch (algo) {
  case IMPL_GPU_TILED:
    mmult_gpu_tiled << <grid, block>>> (gpu_A, gpu_B, gpu_C, matrix_size);
    cudaMemcpy(host_C, gpu_C, alloc_size, cudaMemcpyDeviceToHost);
    break;
  case IMPL_GPU_SIMPLE:
    mmult_gpu_simple << <grid, block>>> (gpu_A, gpu_B, gpu_C, matrix_size);
    cudaMemcpy(host_C, gpu_C, alloc_size, cudaMemcpyDeviceToHost);
    break;
  case IMPL_HOST:
    // memset(host_C, 0, alloc_size);
    simple_mmult(host_A, host_B, host_C, matrix_size);
    break;
  }

  mdump(host_C, matrix_size, "mtx_c.csv");
  cudaFree(gpu_A);
  cudaFree(gpu_B);
  cudaFree(gpu_B);
  free(host_A);
  free(host_B);
  free(host_C);
}
