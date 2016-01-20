#ifndef WIDGET_H
#define WIDGET_H

#include "time.h"
#include <QWidget>

#include "core.h"

namespace Ui {
class Widget;
}


class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void setArgs(int argc, char* argv[]); //определяет, что это будет за программа

    void initGUI();

    void disableGUI();

    void timerEvent(QTimerEvent *t);

private slots:
    void on_start_clicked();

    void on_pushButton_clicked();

    void on_send_clicked();

private:
    Ui::Widget *ui;

    Core* core;

    int timer; //id таймера
    int period; // частота таймера

    // режим программы
    bool normalProgram;
    bool invisProgram;
    bool clanProgram;
    bool exploreProgram;
    bool launcher;
};

#endif // WIDGET_H
