//
// Created by user on 21.07.2021.
//

#include "Timer.h"

TTimer::TTimer(const TTimer::TInterval &value, const TTimerFunction &fun):
    interval(value)
{
    if(fun) OnTimer.connect(fun);
}

void TTimer::SetInterval(const TTimer::TInterval &value)
{
    interval = value;
}

TTimer::TInterval TTimer::Interval() const
{
    return interval;
}

void TTimer::Start()
{
    isOn = true;
}

void TTimer::Stop()
{
    isOn = false;
}

void TTimer::Single()
{
    isSingle = true;
    Start();
}

void TTimer::TimerFunction()
{
    OnTimer();
    if(isSingle)
    {
        Stop();
        isSingle = false;
    }
}

void TTimer::Restart()
{
    if(isOn) Stop();
    Start();
}


