#ifndef CONVOLUTION2D_HH
#define CONVOLUTION2D_HH

#include <algorithm>

/*
 * Note that most classes are implemented directly in this header
 * to allow inlining
 */

template <typename T>
class Kernel {
protected:
    T *mData;
    size_t mWidth;
    size_t mHeight;
    T mSum;

    inline void updateSum() {
        for (size_t i = 0; i < mWidth * mHeight; i++) {
            mSum += mData[i];
        }
    }
public:
    Kernel(T *data, size_t width, size_t height) :
        mData(new T[width * height]),
        mWidth(width), mHeight(height), mSum(0)
    {
        std::copy(data, data + width * height, mData);
        updateSum();
    }

    ~Kernel() {
        delete[] mData;
    }

    inline size_t width() const {
        return mWidth;
    }

    inline size_t height() const {
        return mHeight;
    }

    inline T sum() const {
        return mSum;
    }

    inline T operator[](size_t idx) {
        return mData[idx];
    }
};

/*
 * This is the default ArrayAdaptor implementation for POD types
 *
 */

template<typename T>
class SimpleArrayAdaptor {
protected:
	T *mData;
	size_t mWidth;
	size_t mHeight;

public:
	typedef T ItemType;
	static const inline T Zero() {
		return 0;
	}

	SimpleArrayAdaptor(T* data, size_t width, size_t height)
		: mData(data), mWidth(width), mHeight(height) {}

	inline size_t height() const {
		return mHeight;
	}

	inline size_t width() const {
		return mWidth;
	}

	inline ItemType get(size_t rowIndex, size_t columnIndex) {
		return mData[rowIndex * mWidth + columnIndex];
	}

	inline void set(size_t rowIndex, size_t columnIndex, ItemType value) {
		mData[rowIndex * mWidth + columnIndex] = value;
	}
};

/*
 * Convolution2D must be instantiated with an instance of an Adaptor
 * Adaptor must provide the following interface
 * class Adaptor {
 * public:
 * typedef ... ItemType;
 * ItemType Zero();
 * ItemType get(size_t rowIndex, size_t columnIndex);
 * void set(size_t rowIndex, size_t columnIndex, ItemType value);
 * }
 *
 * ItemType must provide the addition, division, multiplication
 * and assignment operators
 */

class Convolution2D
{
public:
    virtual ~Convolution2D() {}
    virtual void convolve() = 0;
};

template <typename T, typename Adaptor>
class DirectConvolution2D : public Convolution2D {
protected:
    Kernel<T> &mKernel;
    Adaptor &mArrayAdaptor;

public:
    DirectConvolution2D(Kernel<T> &kernel, Adaptor &adaptor) :
        mKernel(kernel),
        mArrayAdaptor(adaptor) {}

    void convolve() {
        size_t kern_height = mKernel.height();
        size_t kern_width = mKernel.width();

        size_t kern_row_off = kern_height >> 1;
        size_t kern_col_off = kern_width >> 1;

        size_t height = mArrayAdaptor.height();
        size_t width = mArrayAdaptor.width();

        T sum = mKernel.sum();

        for (size_t img_row = 0; img_row < height; img_row++) {
            for (size_t img_col = 0; img_col < width; img_col++) {
                typename Adaptor::ItemType accumulator
					= Adaptor::Zero();

                for (size_t kern_row = 0; kern_row < kern_height; kern_row++)
                {
                    int kern_row_idx = kern_height - kern_row - 1;
                    int img_row_idx = img_row + kern_row_idx - kern_row_off;

                    //linear convolution : do not wrap around the border
                    if (img_row_idx < 0 || (size_t)img_row_idx >= height) {
                        continue;
                    }

                    for (size_t kern_col = 0; kern_col < kern_width; kern_col++)
                    {
                        int kern_col_idx = kern_width - kern_col - 1;
                        int img_col_idx = img_col + kern_col_idx - kern_col_off;

                        if (img_col_idx < 0 || (size_t)img_col_idx >= width) {
                            continue;
                        }

                        size_t kern_idx =
                                kern_row_idx * kern_width + kern_col_idx;
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
};

#endif // CONVOLUTION2D_HH
