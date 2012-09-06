#ifndef CONVOLUTION2D_H
#define CONVOLUTION2D_H

#include <QImage>
#include <QTableWidget>

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
        updateSum();
    }

    Kernel(QTableWidget &table) :
        mData(new int[table.columnCount() * table.rowCount()]),
        mWidth(table.columnCount()),
        mHeight(table.rowCount()),
        mSum(0)
    {
        for (size_t row = 0; row < mHeight; row++) {
            size_t row_offset = row * mWidth;

            for (size_t col = 0; col < mWidth; col++) {
                int val = 0;

                QTableWidgetItem *item = table.item(row, col);
                if (item) {
                    QString str = item->text();
                    if (str.length()) {
                        val = str.toInt();
                    }
                }

                mData[row_offset + col] = val;
            }
        }
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

class QRgbItemType {
    friend class QImageArrayAdaptor;
    friend class QImageRawArrayAdaptor;
protected:
    QRgb mRed;
    QRgb mGreen;
    QRgb mBlue;
public:
    inline QRgbItemType() : mRed(0), mGreen(0), mBlue(0) {}
    inline QRgbItemType(QRgb red, QRgb green, QRgb blue):
        mRed(red), mGreen(green), mBlue(blue) {}

    inline QRgbItemType operator+(QRgbItemType other) {
        return QRgbItemType(
            mRed + other.mRed,
            mGreen + other.mGreen,
            mBlue + other.mBlue
        );
    }

    template <typename T>
    inline QRgbItemType operator/(T other) {
        return QRgbItemType(
            reinterpret_cast<QRgb>(mRed / other),
            reinterpret_cast<QRgb>(mGreen / other),
            reinterpret_cast<QRgb>(mBlue / other)
        );
    }

    template <typename T>
    inline QRgbItemType operator*(T other) {
        return QRgbItemType(
            reinterpret_cast<QRgb>(mRed * other),
            reinterpret_cast<QRgb>(mGreen * other),
            reinterpret_cast<QRgb>(mBlue * other)
        );
    }

    inline QRgbItemType operator=(QRgbItemType other) {
        mRed = other.mRed;
        mGreen = other.mGreen;
        mBlue = other.mBlue;
        return *this;
    }
};

class QImageArrayAdaptor {
protected:
    QImage &mImage;

public:
    typedef QRgbItemType ItemType;
    QImageArrayAdaptor(QImage &image) : mImage(image) {}

    inline size_t height() const {
        return mImage.height();
    }

    inline size_t width() const {
        return mImage.width();
    }

    inline ItemType get(size_t rowIndex, size_t columnIndex) {
        QRgb rgb = mImage.pixel(columnIndex, rowIndex);
        return ItemType(qRed(rgb), qGreen(rgb), qBlue(rgb));
    }

    inline void set(size_t rowIndex, size_t columnIndex, ItemType value) {
        QRgb rgb = qRgb(value.mRed, value.mGreen, value.mBlue);
        mImage.setPixel(columnIndex, rowIndex, rgb);
    }
};

class QImageRawArrayAdaptor {
protected:
    QImage &mImage;
    size_t mWidth;
    size_t mHeight;
    const uchar *mBits;
    uchar *mOut;

public:
    typedef QRgbItemType ItemType;

    QImageRawArrayAdaptor(QImage &image, uchar *out) :
        mImage(image), mWidth(image.width()), mHeight(image.height()),
        mBits(image.bits()), mOut(out) {}

    inline size_t height() const {
        return mHeight;
    }

    inline size_t width() const {
        return mWidth;
    }

    inline ItemType get(size_t rowIndex, size_t columnIndex) {
        const uchar *ptr = mBits + 3 * (rowIndex * mWidth + columnIndex);
        QRgb rgb = qRgb(ptr[0], ptr[1], ptr[2]);
        return ItemType(qRed(rgb), qGreen(rgb), qBlue(rgb));
    }

    inline void set(size_t rowIndex, size_t columnIndex, ItemType value) {
        uchar *optr = mOut + 3 * (rowIndex * mWidth + columnIndex);
        optr[0] = value.mRed;
        optr[1] = value.mGreen;
        optr[2] = value.mBlue;
    }
};

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
                typename Adaptor::ItemType accumulator;

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

#endif // CONVOLUTION2D_H
