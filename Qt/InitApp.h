//
// Created by user on 13.11.2021.
//

#include <QApplication>
#include "AppQt.h"
using TApplication = TAppQt;

int QtMain();
int main(int argc, char *argv[])
{
    QApplication app(argc, argv, true);
    return QtMain();
}
int QtMain()

