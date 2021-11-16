//
// Created by user on 09.01.2021.
//

#ifndef ATHENA_APPQT_H
#define ATHENA_APPQT_H

#include "../App.h"
#include "MainWindowQt.h"
#include <QProgressBar>
#include <QMenuBar>
#include <QToolBar>
#include <QPushButton>

class TAppQt : public TApp{
public:
    TAppQt(const TString& nameApp = APP_NAME, const TString& titleApp = TString());
    ~TAppQt();

    TMainWindow* MainWindow() { return mainWindow.get(); }

    void SetTitle(const TString& value) override;
    void SetSecondTitle(const TString& value) override;
    int Run() override;
    bool Close() override;
    TResult ShowMessage(const TString& message, bool isQuestion, bool isCancel) override;
    TResult InputText(const TString& message, TString& value) override;
    TResult InputItems(const TString& message, const TVecString& items, TString& value, int& index, bool isEdit) override;
    TResult SelectFile(const TString &filter, const TString &ext, TString &res) override;
    TPtrProgress GetProgress() override;

    TPtrTimer CreateTimer(const TTimer::TInterval& interval, const TTimerFunction& fun) override;
private:
    std::unique_ptr<TMainWindow> mainWindow;
    QMenuBar* barMenu;
    QToolBar* barTool;
    void UpdateBars();
    void AddMenu(QMenu* menu, TMenuItem& item);
    QIcon IconFromIndex(size_t index);

    QString AllTitle() const;
};

class TSender : public QObject{
public:
    TSender(QObject *parent=nullptr):QObject(parent){}
    Q_SIGNAL void SendValue();
    Q_SIGNAL void FinishValue();
    Q_OBJECT
};

class TProgressQt : public QWidget, public TProgress{
public:
    TProgressQt(QWidget *parent);

    Q_OBJECT
protected:
    QProgressBar* bar;
    QPushButton* cancel;
    //вызывается в дочерем потоке
    void ViewShow() override;
    void CallResult() override;
    //функции вызываемые в главном потоке
    Q_SLOT void SendProgress();
    Q_SLOT void SendResult();
    std::unique_ptr<TSender> sender;
};

#endif //ATHENA_APPQT_H
