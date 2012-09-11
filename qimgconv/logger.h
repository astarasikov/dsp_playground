#ifndef LOGGERWIDGET_H
#define LOGGERWIDGET_H

#include <QTextEdit>

class Logger
{
protected:
    QTextEdit &mTextEdit;
    qint64 mTimeStart;

    const unsigned maxLines = 100;

public:
    Logger(QTextEdit &textEdit) :
        mTextEdit(textEdit),
        mTimeStart(QDateTime::currentMSecsSinceEpoch()) {}

    inline void message(QString &msg) {
        qint64 msec = QDateTime::currentMSecsSinceEpoch();
        QString dt = QString::number(msec - mTimeStart);
        QString timedMessage = QString("%1 %2").arg(dt, msg);
        mTextEdit.append(timedMessage);
    }

    inline void message(const char *str) {
        QString msg(str);
        message(msg);
    }
};

#endif // LOGGERWIDGET_H
