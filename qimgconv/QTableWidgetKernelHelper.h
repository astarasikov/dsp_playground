#ifndef QTABLEWIDGETKERNELHELPER_H
#define QTABLEWIDGETKERNELHELPER_H

#include <QTableWidget>
#include "convolution2d.h"

Kernel<int> kernelFromQTableWidget(const QTableWidget &table) {
    size_t width = table.columnCount();
    size_t height = table.rowCount();
    int data[width * height];
    for (size_t row = 0; row < height; row++) {
        size_t row_offset = row * width;

        for (size_t col = 0; col < width; col++) {
            int val = 0;

            QTableWidgetItem *item = table.item(row, col);
            if (item) {
                QString str = item->text();
                if (str.length()) {
                    val = str.toInt();
                }
            }

            data[row_offset + col] = val;
        }
    }
    return Kernel<int>(data, width, height);
}

#endif // QTABLEWIDGETKERNELHELPER_H
