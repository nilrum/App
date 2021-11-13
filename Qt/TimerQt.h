//
// Created by user on 21.07.2021.
//

#ifndef ATLAS_TIMERQT_H
#define ATLAS_TIMERQT_H

#include "../Timer.h"

class QTimer;

class TTimerQt : public TTimer{
public:
    TTimerQt(const TInterval& value = TInterval{0}, const TTimerFunction& fun = TTimerFunction());
    ~TTimerQt();
    void Start() override;
    void Stop() override;

private:
    QTimer* timer;
};


#endif //ATLAS_TIMERQT_H
