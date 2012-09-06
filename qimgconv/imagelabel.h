#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QResizeEvent>
#include <QDebug>

class ImageLabel : public QLabel {
    Q_OBJECT

protected:
    QPixmap mLastPixmap;

    void __resize(const QSize &size) {
        QLabel::resize(size);

        if (mLastPixmap.isNull()) {
            return;
        }

        size_t width = size.width();
        size_t height = size.height();

        QLabel::setPixmap(mLastPixmap.scaled(width, height,
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));
    }

public:
    ImageLabel() : mLastPixmap(QPixmap()) {}
    ImageLabel(QWidget *parent = 0)
        : QLabel::QLabel(parent), mLastPixmap(QPixmap()) {}

    void setPixmap(const QPixmap &pixmap) {
        mLastPixmap = pixmap;
        __resize(size());
    }

public slots:
    virtual void resize(const QSize &size) {
        __resize(this->size());
    }

    virtual void resizeEvent(QResizeEvent *evt) {
        evt->accept();
        __resize(size());
    }
};

#endif // IMAGELABEL_H
