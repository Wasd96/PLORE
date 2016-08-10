#ifndef WIDGET_H
#define WIDGET_H

#include "time.h"
#include <QWidget>
#include <QLabel>
#include <QTextEdit>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QListWidgetItem>

#include <QShowEvent>

#include "core.h"
#include "particles.h"

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
    void setAlive(int norm, int user, int bot);
    void educate();
    void addTextField();

    void timerEvent(QTimerEvent *t);
    void paintEvent(QPaintEvent *pEv);
    void mouseMoveEvent(QMouseEvent *mEv);
    void mousePressEvent(QMouseEvent *mEv);
    void mouseReleaseEvent(QMouseEvent *mEv);

private slots:
    void died(int type); // прием лаунчера о смерти

    void changeVisible(bool vis);
    void on_start_clicked();
    void on_attack_clicked();
    void on_help_clicked();
    void on_request_clicked();
    void on_find_state_toggled(bool checked);
    void on_up_c_clicked();
    void on_up_d_clicked();
    void on_up_i_clicked();
    void on_launcherTab_currentChanged(int index);
    void on_connections_currentRowChanged(int currentRow);
    void on_attack_count_returnPressed();
    void on_help_count_returnPressed();

private:
    Ui::Widget *ui;
    QLabel* bord;

    Core* core;
    Connection *connection;

    QString name; // имя программы, запуск именно такой проги, а не PLORE.exe
    Particles particles;

    int timer; //id таймера
    int timerIncrease; // id таймера прироста
    int deathTimer; // таймер для красивой смерти
    int period; // частота таймера операций

    // режим программы
    bool normalProgram;
    bool userProgram;
    bool invisProgram;
    bool silentProgram;
    bool troyanProgram;
    bool wormProgram;
    bool exploreProgram;
    bool launcher;
    bool timerProgram;
    bool educateProgram;

    int education; // этап обучения

    int normAlive; // живые норм проги
    int botAlive; // живые боты
    int userAlive; // живые юзеры

    int level; // текущий уровень
    int maxLevel; // максимальный доступный уровень

    bool moving; // флаг-костыль для перемещения окон
    int movingX, movingY; // смещения от угла

    int reviveTimer; // таймер восстановления
    int reviveCountdown; // время восстановления
    QWidget* revWid; // указатель на воскрешаемый виджет
    int revWidAnalog; // номер модуля, невидимые у неюзера
    int realD; // сохранение реального быстродействия
    bool signedModules[9]; // уже описанные повреждения


    QList <QTextEdit*> texts;
};

#endif // WIDGET_H
