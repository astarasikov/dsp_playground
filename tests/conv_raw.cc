#include <cstring>
#include <cstdlib>
#include <iostream>

#include "../timelog.hh"

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

template <typename T>
class DirectConvolution2D {
protected:
	T* mKernel;
	size_t mKernWidth;
	size_t mKernHeight;

	T* mData;
	size_t mWidth;
	size_t mHeight;

	T* mOutData;
	T mKernSum;
public:
    DirectConvolution2D(T *kernel, size_t kern_width, size_t kern_height,
		T *data, size_t width, size_t height,
		T *outData)
		: mKernel(kernel), mKernWidth(kern_width), mKernHeight(kern_height),
		mData(data), mWidth(width), mHeight(height),
		mOutData(outData)
		{
			T kernelSum = 0;
			for (size_t i = 0; i < kern_width * kern_height; i++) {
				kernelSum += kernel[i];
			}
			mKernSum = kernelSum;
		}

    void convolve() {
        size_t kern_height = mKernHeight;
        size_t kern_width = mKernWidth;

        size_t kern_row_off = kern_height >> 1;
        size_t kern_col_off = kern_width >> 1;

        size_t height = mHeight;
        size_t width = mWidth;

        T sum = mKernSum;

        for (size_t img_row = 0; img_row < height; img_row++) {
			size_t img_in_off = img_row * width;
            for (size_t img_col = 0; img_col < width; img_col++) {
                T accumulator = 0;

                for (size_t kern_row = 0; kern_row < kern_height; kern_row++)
                {
                    int kern_row_idx = kern_height - kern_row - 1;
                    int img_row_idx = img_row + kern_row_idx - kern_row_off;

                    //linear convolution : do not wrap around the border
                    if (img_row_idx < 0 || (size_t)img_row_idx >= height) {
                        continue;
                    }

					size_t kern_off = kern_row_idx * kern_width;
					size_t img_off = img_row_idx * width;

                    for (size_t kern_col = 0; kern_col < kern_width; kern_col++)
                    {
                        int kern_col_idx = kern_width - kern_col - 1;
                        int img_col_idx = img_col + kern_col_idx - kern_col_off;

                        if (img_col_idx < 0 || (size_t)img_col_idx >= width) {
                            continue;
                        }

                        size_t kern_idx = kern_off + kern_col_idx;
						size_t img_idx = img_off + img_col_idx;
						T current = mData[img_idx];
						T current_mult = current * mKernel[kern_idx];
                        accumulator = accumulator + current_mult;
                    }
                }

                if (sum != 0) {
                    accumulator = accumulator / sum;
                }

				size_t img_in_idx = img_in_off + img_col;
				mOutData[img_in_idx] = accumulator;
            }
        }
    }
};

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

	DirectConvolution2D<TestType>
		convolution(kernelData, KERNEL_SIZE, KERNEL_SIZE,
			in, SIZE, SIZE, out);
	
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
