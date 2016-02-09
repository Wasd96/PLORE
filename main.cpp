#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    // система подставных данных, для дебага
    /*int argcChange = 5;
    char* argvChange[] = {"plore.name", "bot", "3", "hidden", "silent"};*/
    w.setArgs(argc, argv); //установка типа и параметров
    w.show();

    return a.exec();
}
