//
// Created by user on 21.07.2021.
//

#include "TimerQt.h"
#include <QTimer>


TTimerQt::TTimerQt(const TInterval& value, const TTimerFunction& fun): TTimer(value, fun)
{
    timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout, [this](){ TimerFunction(); });
}

TTimerQt::~TTimerQt()
{
    timer->stop();
    delete timer;
}

void TTimerQt::Start()
{
    TTimer::Start();
    if(timer->isActive()) timer->stop();
    timer->start(std::chrono::duration_cast<std::chrono::milliseconds>(interval));
}

void TTimerQt::Stop()
{
    timer->stop();
    TTimer::Stop();
}

