#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mFileName("")
{
    ui->setupUi(this);
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(loadImage()));

#define FMT(X) {X, #X}
    struct {
        QImage::Format fmt;
        char *name;
    } formats[] = {
        FMT(QImage::Format_ARGB32),
        FMT(QImage::Format_RGB16),
        FMT(QImage::Format_RGB32),
        FMT(QImage::Format_RGB444),
        FMT(QImage::Format_RGB555),
        FMT(QImage::Format_RGB666),
        FMT(QImage::Format_RGB888),
        FMT(QImage::Format_Mono),
        FMT(QImage::Format_ARGB32_Premultiplied),
        FMT(QImage::Format_ARGB4444_Premultiplied),
        FMT(QImage::Format_ARGB6666_Premultiplied),
        FMT(QImage::Format_ARGB8555_Premultiplied),
        FMT(QImage::Format_ARGB8565_Premultiplied)
    };
    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); i++) {
        ui->formatCombo->addItem(formats[i].name, formats[i].fmt);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::loadImage(void) {
    mFileName = QFileDialog::getOpenFileName(
                NULL, trUtf8("Load a binary"),
                QDir::homePath(), "*");

    ui->filePathLabel->setText(mFileName);
}

void MainWindow::on_fireButton_clicked()
{
    QFile file(mFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(NULL, "Dang", "We're doomed");
        return;
    }
    mBuffer = file.readAll();
    file.close();

    unsigned width = ui->widthBox->value();
    unsigned height = ui->heightBox->value();
    unsigned offset = ui->offsetLine->text().toInt(0, 16);

    qDebug() << width << " " << height << " " << offset;

    uchar *data = (uchar*)mBuffer.constData() + offset;

    QImage::Format fmt = (QImage::Format)
            ui->formatCombo->itemData(ui->formatCombo->currentIndex()).toInt();
    mImage = QImage(data, width, height, fmt);
    mPixmap = QPixmap::fromImage(mImage);

    ui->thePicture->setPixmap(mPixmap);
}

