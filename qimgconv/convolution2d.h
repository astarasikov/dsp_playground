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
#endif // CONVOLUTION2D_H
