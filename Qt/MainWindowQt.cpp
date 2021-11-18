//
// Created by user on 11.01.2021.
//

#include "MainWindowQt.h"
#include "AppQt.h"
#include <QCloseEvent>

#include <QToolBar>
#include <QPainter>
#include <QMenu>

void TMainWindow::closeEvent(QCloseEvent *event)
{
    bool res = false;
    OnClose(res);
    if(res == false)
    {
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}

TVerticalButton *TMainWindow::Dock(QDockWidget *value)
{
    if(firstDock == nullptr)
    {
        firstDock = value;
        addDockWidget(Qt::RightDockWidgetArea, value);
    }
    else
    {
        if(secondDock == nullptr)
        {
            secondDock = value;
            addDockWidget(Qt::RightDockWidgetArea, value);
        }
        else
            tabifyDockWidget(secondDock, value);
    }
    return AddButton();
}

void TMainWindow::UnDock(QDockWidget *value)
{
    if(value == firstDock)
        firstDock = nullptr;
    else if(value == secondDock)
        secondDock = nullptr;
    removeDockWidget(value);
}

TVerticalButton *TMainWindow::AddButton()
{
    if(dockBar == nullptr)
    {
        dockBar = new QToolBar("Tabs", this);
        dockBar->setOrientation(Qt::Vertical);
        addToolBar(Qt::RightToolBarArea, dockBar);
    }
    auto res = new TVerticalButton("", dockBar);
    dockBar->addWidget(res);
    return res;
}

//---------------------------------------------------------------------------------------------------------------
void ConnectPopup(QWidget* qtWidget, TWidget* widget)
{
    qtWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(qtWidget, &QWidget::customContextMenuRequested,
            [qtWidget, widget](const QPoint &pos)
            {
                widget->OnCheckPopup();//вызываем сначало проверку доступности
                QMenu menu;
                if(LoadQtMenu(widget->Popup(), menu))        //если меню заполнилось
                    menu.exec(qtWidget->mapToGlobal(pos));  //то отображаем его
            }
    );
}
//---------------------------------------------------------------------------------------------------------------
TVerticalButton::TVerticalButton(const QString &text, QWidget *parent): QPushButton(parent), thisText(text)
{

}

void TVerticalButton::paintEvent(QPaintEvent * event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.rotate(90);
    painter.drawText( 4, -4, thisText);
}

QSize TVerticalButton::sizeHint() const
{
    return GetTextSize();
}

QSize TVerticalButton::minimumSizeHint() const
{
    return GetTextSize();
}

QSize TVerticalButton::GetTextSize() const
{
    QRect r = fontMetrics().boundingRect(thisText);
    return QSize(r.height() + 4 , r.width() + 8);
}

void TVerticalButton::SetThisText(const QString& value)
{
    thisText = value;
    if(text() == "") setText(" "); else setText("");
}

//----------------------------------------------------------------------------------------------------------------------

bool LoadQtMenu(TMenuItem &popup, QMenu &menu)
{
    bool res = false;
    for(size_t i = 0; i < popup.CountItems(); i++)
    {
        TMenuItem& it = popup.Item(i);
        it.RunCheckEnable();//если есть проверка то вызовем ее
        if(it.IsEnabled())
        {
            if(it.CountItems() == 0)
            {
                menu.addAction(TRANSR(it.Text()), it.Call());
                res = true;
            }
            else
                res |= LoadQtMenu(it, *menu.addMenu(TRANSR(it.Text())));
        }
    }
    return res;
}

TChildWidget::TChildWidget(QWidget *parent, Qt::WindowFlags f): QWidget(parent, f)
{
    if(parent == nullptr)
        mainWindow = APP->CustMainWindow<TMainWindow*>();
};