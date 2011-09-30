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

class DspWidget : public QWidget {
	Q_OBJECT
public:
	DspWidget(QWidget *parent = 0);
	void loadImage(QString filename);
protected:
	static const int defaultKernelSize = 3;
	static const int maxKernelSize = 21;
	QLabel *image;
	QLabel *result;
	QSlider *slider_h;
	QSlider *slider_w;
	QTableWidget *kernelTable;
	void createControls(QWidget *);
	QImage *inputImage;
	QImage *outputImage;
protected slots:
	void setKernelWidth(int);
	void setKernelHeight(int);
	void resetKernel(void);
	void resetImage(void);
	void convolve(void);
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
