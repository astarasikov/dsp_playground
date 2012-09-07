#include <cstring>
#include <iostream>

#include "../timelog.hh"
#include "../convolution2d.hh"

#define KERNEL_SIZE 3
#define MAX_NUM 100

typedef int TestType;

template <typename T>
static void print2D(T *data, size_t width, size_t height) {
	std::cout << "[" << std::endl;
	for (size_t row = 0; row < height; row++) {
		for (size_t col = 0; col < width; col++) {
			std::cout << data[width * row + col] << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "]" << std::endl;
}

static void runTest(size_t SIZE, bool debug) {
	TestType *in = new TestType[SIZE * SIZE];
	TestType *out = new TestType[SIZE * SIZE];

	TestType kernelData[KERNEL_SIZE * KERNEL_SIZE] = {
		0, 0, 0,
		0, 1, 0,
		0, 0, 0,
	};

	srand(time(0));
	for (size_t i = 0; i < SIZE * SIZE; i++) {
		in[i] = rand() % MAX_NUM;
	}

	Kernel<TestType> kernel(kernelData, KERNEL_SIZE, KERNEL_SIZE);

	SimpleArrayAdaptor<TestType> adaptor(in, SIZE, SIZE, out);
	DirectConvolution2D<TestType, SimpleArrayAdaptor<TestType> >
		convolution(kernel, adaptor);
	
	std::string title = "2D convolution";
	DefaultTimeLog log(title);
	convolution.convolve();
	log.stop();

	if (debug) {
		print2D(in, SIZE, SIZE);
		print2D(out, SIZE, SIZE);
	}

	delete[] in;
	delete[] out;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " image_size [-debug]" << std::endl;
		return -1;
	}

	size_t count = 0;
	bool debug = false;

	count = atoi(argv[1]);
	std::cout << count << std::endl;

	if (argc >= 3 && !strcmp(argv[2], "-debug")) {
		debug = true;
	}

	runTest(count, debug);

	return 0;
}
