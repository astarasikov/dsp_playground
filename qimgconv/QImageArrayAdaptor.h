#ifndef QIMAGEARRAYADAPTOR_H
#define QIMAGEARRAYADAPTOR_H

#include <QImage>

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
    static const inline QRgbItemType Zero() {
        return QRgbItemType();
    }

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
    static const inline QRgbItemType Zero() {
        return QRgbItemType();
    }

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

#endif // QIMAGEARRAYADAPTOR_H
