#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "connection.h"

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

private slots:
    void on_start_clicked();

    void on_pushButton_clicked();

private:
    Ui::Widget *ui;

    Connection* connection;

    // характеристики
    int* I;
    double* D;
    char* C;




    // режим программы
    bool normalProgram;
    bool invisProgram;
    bool clanProgram;
    bool exploreProgram;
    bool launcher;
};

#endif // WIDGET_H
