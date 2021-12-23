//
// Created by user on 11.01.2021.
//

#ifndef ATHENA_MAINWINDOWQT_H
#define ATHENA_MAINWINDOWQT_H

#include <QMainWindow>
#include <QPushButton>
#include <QDockWidget>
#include <QTabBar>
#include <QVariant>
#include <QTimer>
#include <sigslot/signal.hpp>
#include "Types.h"
#include "../App.h"

class TVerticalButton;
class TMainWindow;

class TMainWindow : public QMainWindow, public TNativeMainWindow{
public:
    using TOnClose = sigslot::signal<bool&>;
    TOnClose OnClose;

    TVerticalButton* Dock(QDockWidget* value);
    void UnDock(QDockWidget* value);

    TVerticalButton* AddButton();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

    QDockWidget* firstDock = nullptr;
    QDockWidget* secondDock = nullptr;
    QToolBar* dockBar = nullptr;
};

void ConnectPopup(QWidget* qtWidget, TWidget* widget);

class TChildWidget : public QWidget{
public:
    TChildWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

protected:
    TMainWindow* mainWindow = nullptr;
};

class TVerticalButton : public QPushButton{
public:
    Q_OBJECT
public:
    TVerticalButton(const QString& text = QString(), QWidget* parent = nullptr);
    void SetThisText(const QString& value);
protected:
    QString thisText;
    void paintEvent(QPaintEvent*);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSize GetTextSize() const;
};
class TDockClosable;

template<typename TypeWidget>
class TDockWidget : public TChildWidget, public TypeWidget{
public:
    TDockWidget(QWidget* parent);
    ~TDockWidget();

    bool IsVisible() const override;
    void SetIsVisible(bool value) override;

    void SetTitle(const TString& value) override;
    void SetIsClosable(bool value) override;

    void SetFloatingState(bool value);
protected:
    TDockClosable* dock = nullptr;
    TVerticalButton* button = nullptr;
    bool isFloatingState = false;
    void ButtonClicked();

    bool IsDockActive() const;
    void SetDockActive();

    void SetMainWindow(TMainWindow *window);

};

template <typename T>
class TWaitCall{
public:
    using TResultFun = std::function<void(T)>;
    using TRightFun = std::function<void()>;
    TWaitCall();
    TWaitCall(const TResultFun& fun);
    TWaitCall(const TRightFun& fun);
    TWaitCall(TWaitCall<T>&& oth);
    ~TWaitCall();

    TResultFun DelayFun() const { return delayFun; }

    void SetResultFun(const TResultFun& value);
    void SetRightFun(const TRightFun& value);
private:
    std::unique_ptr<QTimer> timer;
    TResultFun callResult;          //функция которая будет вызвана как резальтат действия
    TRightFun callRight;            //функция которая будет вызвана сразу при вызове delayFun
    size_t timeInterval = 1000;
    using TCurrent = std::remove_reference_t<std::remove_const_t<T>>;
    TCurrent currentValue{};
    TResultFun delayFun;
    void CheckCall(T value);
    void TimerCall();
};

//Класс необходим что бы иметь возможность подключиться к сигналу о том что щелкнули по кнопке Close
class TDockClosable : public QDockWidget{
public:
    TDockClosable(const QString &title, QWidget *parent = nullptr):QDockWidget(title, parent){};
    TOnNotify OnDockClose;
protected:
    void closeEvent(QCloseEvent *event) override
    {
        QDockWidget::closeEvent(event);
        OnDockClose();
    }
};

template<typename TypeWidget>
TDockWidget<TypeWidget>::TDockWidget(QWidget *parent):TChildWidget(parent), TypeWidget()
{
    button = new TVerticalButton();
    if(mainWindow)
        SetMainWindow(mainWindow);
    else
        dock = new TDockClosable("");
}

template<typename TypeWidget>
TDockWidget<TypeWidget>::~TDockWidget()
{
    if(dock->parentWidget() == nullptr)
    {
        delete dock;
        delete button;
    }
}

template<typename TypeWidget>
void TDockWidget<TypeWidget>::SetMainWindow(TMainWindow *window)
{
    dock = new TDockClosable(TRANSR(TypeWidget::title), window);
    //подключаемся к событию закрытия Dock
    dock->OnDockClose.connect(
            [this, window]()
            {
                window->UnDock(dock);   //удалим Dock из главного окна
                delete button;                //удалим кнопку с панели инструментов
                TypeWidget::OnClose();        //вызовем событие закрытия виджета
            });
    connect(dock, &QDockWidget::topLevelChanged, [this](bool isFl){ isFloatingState = isFl; });

    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setWidget(this);

    dock->setMinimumSize(200, 200);
    dock->setBaseSize(300, 400);

    //для дока разрешаем если необходимо кнопку закрытия
    if(TypeWidget::isClosable == false)
        dock->setFeatures(dock->features() ^ QDockWidget::DockWidgetClosable);

    button = window->Dock(dock);
    button->SetThisText(TRANSR(TypeWidget::title));
    connect(button, &TVerticalButton::clicked, [this](){ ButtonClicked(); });
}

template<typename TypeWidget>
void TDockWidget<TypeWidget>::SetTitle(const TString &value)
{
    TypeWidget::SetTitle(value);
    dock->setWindowTitle(TRANSR(value));
    button->SetThisText(TRANSR(value));
}

template<typename TypeWidget>
void TDockWidget<TypeWidget>::ButtonClicked()
{
    if(IsVisible() && IsDockActive())
        SetIsVisible(false);  //то скрываем ее
    else
        SetIsVisible(true);   //делаем видимой или активной вкладку
}

template<typename TypeWidget>
bool TDockWidget<TypeWidget>::IsVisible() const
{
    return dock->isVisible();
}

template<typename TypeWidget>
void TDockWidget<TypeWidget>::SetIsVisible(bool value)
{
    dock->setVisible(value);
    if(value && IsDockActive() == false) SetDockActive();
    if(value && isFloatingState && dock->isFloating() == false)
        {
            dock->setFloating(true);
            auto size = dock->baseSize();
            auto p = mainWindow->rect().center() - QPoint(size.width(), size.height()) / 2.;
            dock->setGeometry(p.x(), p.y(), size.width(), size.height());
        }
}

template<typename TypeWidget>
bool TDockWidget<TypeWidget>::IsDockActive() const
{
    return (dock->visibleRegion().isEmpty() == false);
}

template<typename TypeWidget>
void TDockWidget<TypeWidget>::SetDockActive()
{
    bool isFound = false;
    auto listTabBars = dock->parentWidget()->findChildren<QTabBar*>();
    for(const auto bar : listTabBars)
    {
        auto count = bar->count();
        for(int i = 0; i < count; i++)
        {
            QVariant data = bar->tabData(i);
            if(data.data_ptr().data.ptr == dock)
            {
                bar->setCurrentIndex(i);
                isFound = true;
                break;
            }
        }
        if(isFound) break;
    }
}

template<typename TypeWidget>
void TDockWidget<TypeWidget>::SetIsClosable(bool value)
{
    TypeWidget::SetIsClosable(value);
    if(value && dock->features().testFlag(QDockWidget::DockWidgetClosable) == false)
        dock->setFeatures(dock->features() | QDockWidget::DockWidgetClosable);
    else
        if(value == false && dock->features().testFlag(QDockWidget::DockWidgetClosable))
        dock->setFeatures(dock->features() ^ QDockWidget::DockWidgetClosable);
}

template<typename TypeWidget>
void TDockWidget<TypeWidget>::SetFloatingState(bool value)
{
    isFloatingState = value;
}


template <typename T>
TWaitCall<T>::TWaitCall()
{
    delayFun = [this](T value)
            {
                CheckCall(value);
            };
}

template<typename T>
TWaitCall<T>::TWaitCall(const TWaitCall::TResultFun &fun):TWaitCall<T>()
{
    SetResultFun(fun);
}


template<typename T>
TWaitCall<T>::TWaitCall(const TWaitCall::TRightFun &fun):TWaitCall<T>()
{
    SetRightFun(fun);
}

template<typename T>
TWaitCall<T>::TWaitCall(TWaitCall<T>&& oth):TWaitCall()
{
    timeInterval = oth.timeInterval;
    SetRightFun(oth.callRight);
    SetResultFun(oth.callResult);
}

template <typename T>
TWaitCall<T>::~TWaitCall()
{
    if(timer && timer->remainingTime() > 0)
        TimerCall();
}

template <typename T>
void TWaitCall<T>::CheckCall(T value)
{
    currentValue = value;
    if(callRight)
        callRight();
    if(timer)
        timer->start();
}

template <typename T>
void TWaitCall<T>::TimerCall()
{
    if(callResult) callResult(currentValue);
}

template<typename T>
void TWaitCall<T>::SetResultFun(const TWaitCall::TResultFun& value)
{
    callResult = value;
    if(callResult && timer == nullptr)
    {
        timer.reset(new QTimer());
        timer->setTimerType(Qt::PreciseTimer);
        timer->setInterval(timeInterval);
        timer->setSingleShot(true);
        QObject::connect(timer.get(), &QTimer::timeout, [this]() { TimerCall(); });
    }
}

template<typename T>
void TWaitCall<T>::SetRightFun(const TWaitCall::TRightFun &value)
{
    callRight = value;
}


using TWaitCallDouble = TWaitCall<double>;
using TWaitCallInt = TWaitCall<double>;
using TWaitCallQString = TWaitCall<QString>;


bool LoadQtMenu(TMenuItem& popup, QMenu& menu);

#endif //ATHENA_MAINWINDOWQT_H
