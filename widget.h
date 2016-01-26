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

    void disableGUI(); // сокрытие всех элементов интерфейса

    void timerEvent(QTimerEvent *t);

private slots:
    void on_start_clicked();

    void on_pushButton_clicked();

    void on_send_clicked();

    void on_attack_clicked();

    void on_help_clicked();

    void on_connections_itemSelectionChanged();

    void on_request_clicked();

    void on_find_state_toggled(bool checked);

    void on_up_c_clicked();

    void on_up_d_clicked();

    void on_up_i_clicked();

private:
    Ui::Widget *ui;

    Core* core;

    int timer; //id таймера
    int period; // частота таймера

    // режим программы
    bool normalProgram;
    bool userProgram;
    bool invisProgram;
    bool clanProgram;
    bool exploreProgram;
    bool launcher;
};

#endif // WIDGET_H
