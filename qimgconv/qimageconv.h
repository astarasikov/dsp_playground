#ifndef __QIMAGECONV_H__
#define __QIMAGECONV_H__

#include <QApplication>
#include <QMainWindow>

#include <QMenu>
#include <QMenuBar>
#include <QStyle>

#include <QFrame>
#include <QLabel>
#include <QTableWidget>
#include <QSlider>

#include "imagelabel.h"

class DspWidget : public QWidget {
    Q_OBJECT
public:
    DspWidget(QWidget *parent = 0);
    void loadImage(QString filename);
protected:
    static const int defaultKernelSize = 3;
    static const int maxKernelSize = 21;
    QSlider *slider_h;
    QSlider *slider_w;
    QTableWidget *kernelTable;
    void createControls(QWidget *);

    ImageLabel *inputImageDisplay;
    ImageLabel *outputImageDisplay;
    QImage *inputImage;
    QImage *outputImage;
    uchar *outputBuffer;
protected slots:
    void setKernelWidth(int);
    void setKernelHeight(int);
    void resetKernel(void);
    void resetOutputImage(void);
    void replaceOutputImage(QImage *img);
    void convolve(void);
    void convolveFFT(void);
    void refreshImages(void);
    void fillKernel(void);
};

class DspWindow : public QMainWindow {
    Q_OBJECT
public:
    DspWindow(QWidget *parent = 0);
protected:
    DspWidget *dsp;

    void buildMenu(void);
protected slots:
    void openFile(void);
};

#endif // __QIMAGECONV_H__
