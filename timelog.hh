#ifndef TIMELOG_H
#define TIMELOG_H

#include <iostream>
#include <string>

#include <sys/time.h>
#include <unistd.h>

class TimeLog {
public:
    virtual ~TimeLog() {}
    virtual void stop() = 0;
    virtual void message(std::string &str) = 0;
};

class DebugTimeLog {
protected:
    std::string &mLogTitle;
    long long mTimeStart;

    static long long msecNow() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

public:
    DebugTimeLog(std::string &logTitle)
        : mLogTitle(logTitle), mTimeStart(msecNow())
    {
        std::cerr << "Started " << mLogTitle << std::endl;
    }

    ~DebugTimeLog() {

    }
    virtual void stop() {
        long long dt = msecNow() - mTimeStart;
        std::cerr << mLogTitle << " took " << dt << " msec" << std::endl;
    }
};

typedef DebugTimeLog DefaultTimeLog;

#endif // TIMELOG_H
