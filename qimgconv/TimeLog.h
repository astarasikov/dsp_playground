#ifndef TIMELOG_H
#define TIMELOG_H

#include <QDateTime>
#include <QString>

class TimeLog {
public:
    virtual ~TimeLog() {}
    virtual void stop() = 0;
    virtual void message(QString &str) = 0;
};

class DebugTimeLog {
protected:
    QString &mLogTitle;
    qint64 mTimeStart;

    static qint64 msecNow() {
        return QDateTime::currentMSecsSinceEpoch();
    }

public:
    DebugTimeLog(QString &logTitle)
        : mLogTitle(logTitle), mTimeStart(msecNow())
    {
        qDebug() << "Started " << mLogTitle;
    }

    ~DebugTimeLog() {

    }
    virtual void stop() {
        qint64 dt = msecNow() - mTimeStart;
        qDebug() << mLogTitle << " took " << dt << " msec";
    }
};

typedef DebugTimeLog DefaultTimeLog;

#endif // TIMELOG_H
