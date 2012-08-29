#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    QString mFileName;
    QImage mImage;
    QPixmap mPixmap;
    QByteArray mBuffer;

public slots:
    void loadImage(void);
private slots:
    void on_fireButton_clicked();
};

#endif // MAINWINDOW_H
