#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.setArgs(argc, argv); //установка типа и параметров
    w.show();

    return a.exec();
}
