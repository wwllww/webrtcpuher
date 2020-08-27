#include "DemoPusher.h"
#include <QtWidgets/QApplication>
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DemoPusher w;
    w.show();

    return a.exec();
}
