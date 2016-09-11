#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    // система подставных данных, для дебага
    int argcChange = 2;
    char* argvChange[] = {"plore.name", "finish"};
    w.setArgs(argcChange, argvChange); //установка типа и параметров
    w.show();

    return a.exec();
}
