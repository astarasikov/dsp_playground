#include <cstring>
#include <iostream>

#include <omp.h>

#include "../timelog.hh"
#include "../convolution2d.hh"

template <typename T, typename Adaptor>
class ParallelConvolution2D : public Convolution2D {
protected:
    Kernel<T> &mKernel;
    Adaptor &mArrayAdaptor;

	size_t mWidth;
	size_t mHeight;

	size_t mKernelWidth;
	size_t mKernelHeight;

public:
    ParallelConvolution2D(Kernel<T> &kernel, Adaptor &adaptor) :
        mKernel(kernel),
        mArrayAdaptor(adaptor),
		mWidth(mArrayAdaptor.width()),
		mHeight(mArrayAdaptor.height()),
		mKernelWidth(mKernel.width()),
		mKernelHeight(mKernel.height())
		{}

    void convolve(
		size_t row_start, size_t row_end,
		size_t col_start, size_t col_end) {

        size_t kern_row_off = mKernelHeight >> 1;
        size_t kern_col_off = mKernelWidth >> 1;

        T sum = mKernel.sum();

        for (size_t img_row = row_start; img_row < row_end; img_row++) {
			size_t cur_col_start = img_row == row_start ? col_start : 0;
			size_t cur_col_end = img_row == row_end - 1 ? col_end : mWidth;

            for (size_t img_col = cur_col_start;
				img_col < cur_col_end; img_col++)
			{
                typename Adaptor::ItemType accumulator
					= Adaptor::Zero();

                for (size_t kern_row = 0; kern_row < mKernelHeight; kern_row++)
                {
                    int kern_row_idx = mKernelHeight - kern_row - 1;
                    int img_row_idx = img_row + kern_row_idx - kern_row_off;

                    //linear convolution : do not wrap around the border
                    if (img_row_idx < 0 || (size_t)img_row_idx >= mHeight) {
                        continue;
                    }

                    for (size_t kern_col = 0; kern_col < mKernelWidth; kern_col++)
                    {
                        int kern_col_idx = mKernelWidth - kern_col - 1;
                        int img_col_idx = img_col + kern_col_idx - kern_col_off;

                        if (img_col_idx < 0 || (size_t)img_col_idx >= mWidth) {
                            continue;
                        }

                        size_t kern_idx =
                                kern_row_idx * mKernelWidth + kern_col_idx;
                        typename Adaptor::ItemType current =
                                mArrayAdaptor.get(img_row_idx, img_col_idx);

                        typename Adaptor::ItemType current_mult =
                                current * mKernel[kern_idx];

                        accumulator = accumulator + current_mult;
                    }
                }

                if (sum != 0) {
                    accumulator = accumulator / sum;
                }
                mArrayAdaptor.set(img_row, img_col, accumulator);
            }
        }
    }

	void convolve() {
		size_t block_count = 3;
		size_t chunk = mHeight / block_count;

	#if 1
		#pragma omp parallel num_threads(block_count)
		{
			size_t tnum = omp_get_thread_num();
			convolve(chunk * tnum, chunk * (1 + tnum), 0, mWidth);
		}
	#else
		#pragma omp parallel for
		for (size_t i = 0; i < block_count; i++) {
			convolve(chunk * i, chunk * (i + 1), 0, mWidth);
		}
	#endif
		if (mHeight % block_count) {
			convolve(mHeight & ~(block_count - 1),
				mHeight, 0, mWidth);
		}
	}
};

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
	ParallelConvolution2D<TestType, SimpleArrayAdaptor<TestType> >
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
