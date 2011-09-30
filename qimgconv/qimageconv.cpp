#include <QApplication>
#include <QMainWindow>

#include <QFileDialog>
#include <QSizePolicy>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableWidgetItem>

#include "qimageconv.h"

DspWidget::DspWidget(QWidget *parent)
	: inputImage(NULL), outputImage(NULL) {
	Q_UNUSED(parent);

	QFrame *controls = new QFrame(this);
	controls->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	QHBoxLayout *lay_controls = new QHBoxLayout(controls);

	kernelTable = new QTableWidget(this);
	kernelTable->setVerticalHeader(NULL);
	kernelTable->setHorizontalHeader(NULL);
	kernelTable->setGridStyle(Qt::DashDotLine);
	kernelTable->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

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
}

void DspWidget :: convolve(void) {
	if (!inputImage) {
		return;
	}

	if (!outputImage) {
		return;
	}

	int kern_w = kernelTable->columnCount();
	int kern_h = kernelTable->rowCount();

	//origin offset
	int dx = kern_w >> 1;
	int dy = kern_h >> 1;

	double sum = 0;

	int *kernel = new int[kern_w * kern_h];
	for (int i = 0; i < kern_h; i++) {
		for (int j = 0; j < kern_w; j++) {
			int idx = i * kern_w + j;

			kernel[idx] = 0;
			QTableWidgetItem *item = kernelTable->itemAt(i, j);
			if (item) {
				QString str = kernelTable->item(i, j)->text();
				if (str.length() > 0) {
					kernel[idx] = str.toInt();
				}
			}
			sum += kernel[idx];
		}
	}

	int w = outputImage->width();
	int h = outputImage->height();

	for (int i = 0; i < h; i++) {
		//loop in vertical direction
		for (int j = 0; j < w; j++) {
			QRgb _red = 0;
			QRgb _green = 0;
			QRgb _blue = 0;
	
			for (int k = 0; k < kern_h; k++) {
				//flip kernel horizontally
				int kern_i = kern_h - k - 1;

				//shift to origin
				int _k = i + kern_i - dy;
				
				//boundary check
				if (_k < 0 || _k >= h) {
					continue;
				}

				for (int m = 0; m < kern_w; m++) {
					//flip kernel vertically
					int kern_j = kern_w - m - 1;

					//shift to origin
					int _m = j + kern_j - dx;
					
					if (_m < 0 || _m >= w) {
						continue;
					}
					
					int kern_idx = kern_i * kern_w + kern_j;

					QRgb sig = outputImage->pixel(_m, _k);
					QRgb red = qRed(sig);
					QRgb green = qGreen(sig);
					QRgb blue = qBlue(sig);

					_red += red * kernel[kern_idx];
					_green += green * kernel[kern_idx];
					_blue += blue * kernel[kern_idx];
				}
			}
			_red = (QRgb)(_red / sum);
			_green = (QRgb)(_green / sum);
			_blue = (QRgb)(_blue / sum);

			QRgb result = qRgb(_red, _green, _blue);
			outputImage->setPixel(j, i, result);
		}
	}
	
	delete[] kernel;

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
	inputImage = new QImage(filename);
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
