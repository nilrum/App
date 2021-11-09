//
// Created by user on 21.07.2021.
//

#ifndef ATLAS_TIMER_H
#define ATLAS_TIMER_H
#include <functional>
#include <chrono>
#include <sigslot/signal.hpp>

using TTimerFunction = std::function<void()>;

class TTimer {
public:
    using TInterval = std::chrono::milliseconds;

    TTimer(const TInterval& value = TInterval{0}, const TTimerFunction& fun = TTimerFunction());
    virtual ~TTimer(){};

    void SetInterval(const TInterval& value);
    TInterval Interval() const;

    inline bool IsOn() const { return isOn; }

    virtual void Start();
    virtual void Stop();

    void Single();
    void Restart();

    sigslot::signal<> OnTimer;
protected:
    TInterval interval{0};
    bool isSingle = false;
    bool isOn = false;
    void TimerFunction();
};

using TPtrTimer = std::shared_ptr<TTimer>;

#endif //ATLAS_TIMER_H
