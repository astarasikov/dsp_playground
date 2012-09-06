#include <QApplication>
#include <QMainWindow>

#include <QFileDialog>
#include <QSizePolicy>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableWidgetItem>

#include <QDateTime>
#include <QDebug>

#include <complex>

#include "qimageconv.h"
#include "convolution2d.h"
#include "QImageArrayAdaptor.h"
#include "QTableWidgetKernelHelper.h"

#include "TimeLog.h"

#include "../fft.hh"

DspWidget::DspWidget(QWidget *parent)
    : inputImage(NULL), outputImage(NULL) {
    Q_UNUSED(parent);

    QFrame *controls = new QFrame(this);
    controls->setSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::Minimum);
    QHBoxLayout *lay_controls = new QHBoxLayout(controls);

    kernelTable = new QTableWidget(this);
    kernelTable->setVerticalHeader(NULL);
    kernelTable->setHorizontalHeader(NULL);
    kernelTable->setGridStyle(Qt::DashDotLine);
    kernelTable->setSizePolicy(QSizePolicy::MinimumExpanding,
        QSizePolicy::Minimum);

    QFrame *switches = new QFrame(this);
    createControls(switches);
    lay_controls->addWidget(switches);
    lay_controls->addWidget(kernelTable);;

    image = new QLabel(this);
    image->setScaledContents(true);

    result = new QLabel(this);
    result->setScaledContents(true);

    QFrame *images = new QFrame(this);
    images->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QHBoxLayout *hb_images = new QHBoxLayout(images);
    hb_images->addWidget(image);
    hb_images->addWidget(result);

    QVBoxLayout *vb_ui = new QVBoxLayout(this);
    vb_ui->addWidget(controls);
    vb_ui->addWidget(images);
    setLayout(vb_ui);
}

void DspWidget :: createControls(QWidget *frame) {
    QGridLayout *lay_controls = new QGridLayout(frame);
    QLabel *lbl_w = new QLabel(tr("kernel width"));
    slider_w = new QSlider(Qt::Horizontal);
    slider_w->setRange(1, maxKernelSize);
    connect(slider_w, SIGNAL(valueChanged(int)), this, SLOT(setKernelWidth(int)));
    slider_w->setValue(defaultKernelSize);
    lay_controls->addWidget(lbl_w, 0, 0);
    lay_controls->addWidget(slider_w, 0, 1);

    QLabel *lbl_h = new QLabel(tr("kernel height"));
    slider_h = new QSlider(Qt::Horizontal);
    slider_h->setRange(1, maxKernelSize);
    connect(slider_h, SIGNAL(valueChanged(int)), this, SLOT(setKernelHeight(int)));
    slider_h->setValue(defaultKernelSize);
    lay_controls->addWidget(lbl_h, 1, 0);
    lay_controls->addWidget(slider_h, 1, 1);

    QPushButton *bn_reset_kern = new QPushButton(tr("Reset Kernel"));
    connect(bn_reset_kern, SIGNAL(clicked()), this, SLOT(resetKernel()));
    lay_controls->addWidget(bn_reset_kern, 2, 0);

    QPushButton *bn_reset_img = new QPushButton(tr("Reset Image"));
    connect(bn_reset_img, SIGNAL(clicked()), this, SLOT(resetImage()));
    lay_controls->addWidget(bn_reset_img, 3, 0);

    QPushButton *bn_convolve = new QPushButton(tr("Convolve"));
    connect(bn_convolve, SIGNAL(clicked()), this, SLOT(convolve()));
    lay_controls->addWidget(bn_convolve, 4, 0);

    QPushButton *bn_convolveFFT = new QPushButton(tr("Convolve FFT"));
    connect(bn_convolveFFT, SIGNAL(clicked()), this, SLOT(convolveFFT()));
    lay_controls->addWidget(bn_convolveFFT, 5, 0);
}

void DspWidget :: convolveFFT(void) {
    if (!inputImage) {
        return;
    }

    if (!outputImage) {
        return;

    }

    //image dimensions
    int w = outputImage->width();
    int h = outputImage->height();
    int dim = next_power_of_two(w > h ? w : h);

    int chan_size = dim * dim;

    //kernel dimensions
    int user_kern_w = kernelTable->columnCount();
    int user_kern_h = kernelTable->rowCount();

    int kern_w = dim;
    int kern_h = dim;

    auto kernel = new std::complex<double>[kern_w * kern_h];
    Q_ASSERT(kernel != NULL);

    auto image = new std::complex<long double>[chan_size * 3];
    Q_ASSERT(image != NULL);

    double sum = 0;

    for (int i = 0; i < kern_h; i++) {
        for (int j = 0; j < kern_w; j++) {
            int idx = i * kern_w + j;

            kernel[idx] = 0;

            if (i >= user_kern_h || j >= user_kern_w) {
                continue;
            }

            QTableWidgetItem *item = kernelTable->itemAt(i, j);
            if (item) {
                QString str = kernelTable->item(i, j)->text();
                if (str.length() > 0) {
                    kernel[idx] = str.toInt();
                    sum += kernel[idx].real();
                }
            }
        }
    }

    for (int i = 0; i < user_kern_w; i++) {
        int ki = i * kern_w;
        for (int j = 0; j < user_kern_h; j++) {
            kernel[ki + j] /= sum;
        }
    }
    fft_2d(kernel, kern_w, kern_h, false);

    for (int i = 0; i < h; i++) {
        int r_idx = dim * i;
        int g_idx = chan_size + r_idx;
        int b_idx = 2 * chan_size + r_idx;

        for (int j = 0; j < w; j++) {
            QRgb pix = outputImage->pixel(j, i);
            image[r_idx + j] = qRed(pix);
            image[g_idx + j] = qGreen(pix);
            image[b_idx + j] = qBlue(pix);
        }
    }

    fft_2d(image, dim, dim, false);
    fft_2d(image + chan_size, dim, dim, false);
    fft_2d(image + 2 * chan_size, dim, dim, false);

    for (int i = 0; i < dim; i++) {
        int r_pad = dim * i;
        int g_pad = chan_size + r_pad;
        int b_pad = 2 * chan_size + r_pad;
        for (int j = 0; j < dim; j++) {
            image[r_pad + j] *= kernel[r_pad + j];
            image[g_pad + j] *= kernel[r_pad + j];
            image[b_pad + j] *= kernel[r_pad + j];
        }
    }

    //FIXME: perform convolution
    fft_2d(image, dim, dim, true);
    fft_2d(image + chan_size, dim, dim, true);
    fft_2d(image + 2 * chan_size, dim, dim, true);

    for (int i = 0; i < h; i++) {
        int r_idx = dim * i;
        int g_idx = chan_size + r_idx;
        int b_idx = 2 * chan_size + r_idx;

        for (int j = 0; j < w; j++) {
            int r = (int)real(image[r_idx + j]);
            int g = (int)real(image[g_idx + j]);
            int b = (int)real(image[b_idx + j]);
            outputImage->setPixel(j, i, qRgb(r, g, b));
        }
    }

    delete[] kernel;
    refreshImages();
}

void DspWidget :: convolve(void) {
    if (!inputImage) {
        return;
    }

    if (!outputImage) {
        return;
    }

    uchar *out = new uchar[outputImage->byteCount()];

    Kernel<int> kernel = kernelFromQTableWidget(*(this->kernelTable));
    QImageRawArrayAdaptor adaptor(*outputImage, out);
    DirectConvolution2D<int, QImageRawArrayAdaptor>
            convolution(kernel, adaptor);

    QString title("2D Convolution");
    DebugTimeLog log(title);
    convolution.convolve();
    log.stop();

    QImage *newImage = new QImage(
        out,
        outputImage->width(),
        outputImage->height(),
        outputImage->format());

    delete outputImage;
    outputImage = newImage;

    refreshImages();
}

void DspWidget :: fillKernel(void) {
    int kern_w = kernelTable->columnCount();
    int kern_h = kernelTable->rowCount();

    for (int i = 0; i < kern_h; i++) {
        for (int j = 0; j < kern_w; j++) {
            QString str = QString::number(i == j && i == kern_h >> 1 ? 1 : 0);
            kernelTable->setItem(i, j, new QTableWidgetItem(str));
        }
    }
}

void DspWidget :: resetKernel(void) {
    slider_w->setValue(defaultKernelSize);
    slider_h->setValue(defaultKernelSize);
    fillKernel();
}

void DspWidget :: resetImage(void) {
    if (outputImage) {
        delete outputImage;
        outputImage = NULL;
    }
    if (inputImage) {
        outputImage = new QImage(*inputImage);
        refreshImages();
    }
}

void DspWidget :: setKernelWidth(int columns) {
    kernelTable->setColumnCount(columns);
    fillKernel();
}

void DspWidget :: setKernelHeight(int rows) {
    kernelTable->setRowCount(rows);
    fillKernel();
}

void DspWidget :: refreshImages(void) {
    QPixmap in_pm;
    in_pm.convertFromImage(*inputImage);
    image->setPixmap(in_pm);

    QPixmap out_pm;
    out_pm.convertFromImage(*outputImage);
    result->setPixmap(out_pm);
}

void DspWidget :: loadImage(QString filename) {
    if (inputImage) {
        delete inputImage;
    }
    if (outputImage) {
        delete outputImage;
    }
    QImage img(filename);
    inputImage = new QImage(img.convertToFormat(QImage::Format_RGB888));
    outputImage = new QImage(*inputImage);
    refreshImages();
}

DspWindow :: DspWindow(QWidget *parent) {
    Q_UNUSED(parent);

    dsp = new DspWidget();
    buildMenu();
    setCentralWidget(dsp);

    setWindowTitle("Convolution");
}

void DspWindow :: buildMenu(void) {
    QIcon fpic = style()->standardIcon(QStyle::SP_DirIcon);
    QIcon qpic = style()->standardIcon(QStyle::SP_DialogCloseButton);

    QAction *quit = new QAction(qpic, tr("&Quit"), this);
    QAction *open = new QAction(fpic, tr("&Open"), this);

    connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(open, SIGNAL(triggered()), this, SLOT(openFile()));

    QMenu *file = menuBar()->addMenu(tr("&File"));
    file->addAction(open);
    file->addSeparator();
    file->addAction(quit);
}

void DspWindow :: openFile() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Image"),
        QDir::currentPath(), tr("Images (*.jpg *.png *.bmp)"));

    if (!filename.isNull()) {
        dsp->loadImage(filename);
    }
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    DspWindow wnd;

    wnd.resize(320, 240);
    wnd.show();

    return app.exec();
}
