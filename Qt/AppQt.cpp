//
// Created by user on 09.01.2021.
//

#include "AppQt.h"

#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QStatusBar>
#include "TimerQt.h"
#include "../FileSystem/FileSystem.h"


INIT_PROPERTYS(TAppQt)

TAppQt::TAppQt(const TString &nameApp, const TString &titleApp) : TApp(nameApp, titleApp)
{
    auto lay = new QVBoxLayout();
    mainWindow.reset(new TMainWindow());
    qApp->setWindowIcon(QIcon(":/icons/app.png"));
    mainWindow->OnClose.connect([this](bool& res){ bool r = true; OnClose(r); res = r;});
    mainWindow->setCentralWidget(new QWidget(mainWindow.get()));
    mainWindow->centralWidget()->setLayout(lay);

    barMenu = new QMenuBar(mainWindow.get());
    mainWindow->setMenuBar(barMenu);

    barTool = mainWindow->addToolBar("Tools");

    mainWindow->setStatusBar(new QStatusBar(mainWindow.get()));
    mainWindow->statusBar()->setMaximumHeight(23);
    menu->OnChange.connect([this](){ UpdateBars(); });
}

int TAppQt::Run()
{
    BeforeRun();
    mainWindow->setWindowTitle(TRANSR(title));//переводим только после загрузки переводчика
    mainWindow->showMaximized();
    BeforeShowing();
    int res = QApplication::exec();
    AfterRun();
    return res;
}

TResult TAppQt::ShowMessage(const TString &message, bool isQuestion, bool isCancel)
{
    if(isQuestion)
    {
        QMessageBox::StandardButtons buttons = QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No);
        if(isCancel) buttons = buttons | QMessageBox::Cancel;
        QMessageBox box(QMessageBox::Question, TRANSR(title), STR(message), buttons, mainWindow.get());
        box.setButtonText(QMessageBox::Yes, TRANSR("Yes"));
        box.setButtonText(QMessageBox::No, TRANSR("No"));
        if(isCancel) box.setButtonText(QMessageBox::Cancel, TRANSR("Cancel"));
        switch (box.exec())
        {
            case QMessageBox::Yes:      return TMessResult::Yes;
            case QMessageBox::No:       return TMessResult ::No;
            case QMessageBox::Cancel:   return TResult::Cancel;
        }
    }
    else
    {
        QMessageBox::information(mainWindow.get(), TRANSR(title), STR(message));
        return TMessResult::Yes;
    }
    return TMessResult::No;
}

TResult TAppQt::InputText(const TString &message, TString &value)
{
    QString res = QInputDialog::getText(mainWindow.get(), TRANSR(title), STR(message), QLineEdit::Normal, STR(value));
    if(res.isEmpty()) return TResult::Cancel;

    value = res.toStdString();
    return TResult();
}

TResult TAppQt::InputItems(const TString &message, const TVecString &items, TString &value, int &index, bool isEdit)
{
    bool isOk = false;

    QStringList list;
    for(const auto& it : items)
        list.push_back(STR(it));

    bool isExists = true;//флаг который хранит существовал ли элемент в списке
    if(index == -1 && value.empty() == false)
    {
        auto it = std::find(items.begin(), items.end(), value);
        if(it != items.end())
            index = it - items.begin();
        else
        {
            isExists = false;
            list.push_back(STR(value));//добавляем с список для выбора
            index = list.size() - 1;
        }
    }

    QString res = QInputDialog::getItem(mainWindow.get(), TRANSR(title), STR(message), list, index, isEdit, &isOk);

    if(isOk && res.isEmpty() == false)
    {
        value = res.toStdString();

        auto it = std::find(items.begin(), items.end(), value);
        index = (it == items.end()) ? -1 : it - items.begin();
        return TResult();
    }
    return TResult::Cancel;
}

TResult TAppQt::SelectFile(const TString &filter, const TString &ext, TString &res)
{
    QString resQt;
    if(ext.size())
    {
        resQt = QFileDialog::getSaveFileName(mainWindow.get(), TRANSR("Input file name"), STR(lastSaveDir), filter.c_str());
        lastSaveDir = STR(fs::path(resQt.toStdString()).parent_path().string());
    }
    else
    {
        resQt = QFileDialog::getOpenFileName(mainWindow.get(), TRANSR("Select loading file"), STR(lastOpenDir), filter.c_str());
        lastOpenDir = STR(fs::path(resQt.toStdString()).parent_path().string());
    }

    if(resQt.isEmpty())
        return TResult::Cancel;

    res = resQt.toStdString();

    return TResult();
}


void TAppQt::AddMenu(QMenu* menu, TMenuItem& item)
{
    for(size_t i = 0; i < item.CountItems(); i++)
    {
        TMenuItem& child = item.Item(i);
        if (child.CountItems())
            AddMenu(menu->addMenu(TRANSR(child.Text())), child);
        else
        {
            if(child.Text() != "-")
            {
                QAction* ac;
                if(child.Call())
                    ac = menu->addAction(IconFromIndex(child.IndexImg()), TRANSR(child.Text()), child.Call(),
                                                  QKeySequence(STR(child.Shortcut())));
                else
                    ac = menu->addAction(IconFromIndex(child.IndexImg()), TRANSR(child.Text()));

                ac->setEnabled(child.IsEnabled());
                ac->setVisible(child.IsVisible());
                if(child.IsCheckable())
                {
                    ac->setCheckable(child.IsCheckable());
                    ac->setChecked(child.IsChecked());
                    child.OnChecked.connect([ac](bool value) { ac->setChecked(value); });
                }
                child.OnEnabled.connect([ac](bool value) { ac->setEnabled(value); });
            }
            else
                menu->addSeparator();
        }
        if(child.IsToolBar())
        {
            QAction *ac = barTool->addAction(IconFromIndex(child.IndexImg()),
                                             TRANSR(child.Text()), child.Call());
            ac->setEnabled(child.IsEnabled());
            child.OnEnabled.connect([ac](bool value) { ac->setEnabled(value); });
        }
    }
}

void TAppQt::UpdateBars()
{
    barMenu->clear();
    barTool->clear();
    TMenuItem& root = *menu;
    for(size_t i = 0; i < root.CountItems(); i++)
        AddMenu(barMenu->addMenu(TRANSR(root.Item(i).Text())), root.Item(i));
}

static TVecString iconNames = {"open.png", "save.png", "close.png", "add.png", "del.png",
             "tools.png", "new.png", "back.png", "next.png"};

QIcon TAppQt::IconFromIndex(size_t index)
{
    if(index < iconNames.size())
        return QIcon(QPixmap((":icons/" + iconNames[index]).c_str()));
    return QIcon();
}

bool TAppQt::Close()
{
    return mainWindow->close();
}

TAppQt::~TAppQt()
{
    for(auto& p : progresses)
        p->SetCancel();
    TThreadApp::WaitFinishAllThread();
    progresses.clear();
}

void TAppQt::SetTitle(const TString &value)
{
    TAppItem::SetTitle(value);
    mainWindow->setWindowTitle(AllTitle());
}

void TAppQt::SetSecondTitle(const TString &value)
{
    TApp::SetSecondTitle(value);
    mainWindow->setWindowTitle(AllTitle());
}

QString TAppQt::AllTitle() const
{
    return STR(TRANS(title) + secondTitle);
}

TPtrProgress TAppQt::GetProgress()
{
    for(const auto& p : progresses)
        if(p->IsFinished())
        {
            p->Reset();
            return p;
        }

    auto res = std::make_shared<TProgressQt>(mainWindow.get());
    progresses.emplace_back(res);
    mainWindow->statusBar()->addWidget(res.get());
    return res;
}

TPtrTimer TAppQt::CreateTimer(const TTimer::TInterval& interval, const TTimerFunction& fun)
{
    return std::make_shared<TTimerQt>(interval, fun);
}
//----------------------------------------------------------------------------------------------------------------------
TProgressQt::TProgressQt(QWidget *parent) : QWidget(parent)
{
    setLayout(new QHBoxLayout());
    layout()->setMargin(0);

    bar = new QProgressBar(this);
    layout()->addWidget(bar);

    cancel = new QPushButton(this);
    cancel->setText(TRANSR("Cancel"));
    cancel->setMinimumHeight(20);
    connect(cancel, &QPushButton::clicked, [this](){ SetCancel(); });
    layout()->addWidget(cancel);

    sender = std::make_unique<TSender>();
    connect(sender.get(), &TSender::SendValue, this, &TProgressQt::SendProgress, Qt::QueuedConnection);
    connect(sender.get(), &TSender::FinishValue, this, &TProgressQt::SendResult, Qt::QueuedConnection);
}

void TProgressQt::ViewShow()
{
    sender->SendValue();
}

void TProgressQt::CallResult()
{
    sender->FinishValue();
}

void TProgressQt::SendProgress()
{
    TLock guard(mut);

    if(isChanged)
    {
        bar->setMaximum(maxProg);
        if(TProgress::text.size())
            bar->setFormat(STR(Text() + " %p%"));
        isChanged = false;
    }

    if(cur >= maxProg || cur == -1)
    {
        hide();
        return;
    }
    if(isVisible() == false)
        show();

    bar->setValue(cur);
}

void TProgressQt::SendResult()
{
    OnResult(result);
}

