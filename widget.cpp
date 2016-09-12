#include "widget.h"
#include "ui_widget.h"
#include <QProcess>
#include <QDesktopWidget>
#include <QDebug>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    srand(time(NULL)*(int)(ui)); // зависимость зерна от адреса GUI,
                                 // т.к. процессы запускаются практически одновременно

    // инициализация типов программ
    launcher = false;
    userProgram = false;
    normalProgram = false;
    invisProgram = false;
    silentProgram = false;
    troyanProgram = false;
    wormProgram = false;
    exploreProgram = false;
    timerProgram = false;
    educateProgram = false;

    education = 0;
    level = 0;
    timer = 0;
    timerIncrease = 0;
    deathTimer = -1;
    maxLevel = 0;

    userAlive = 0;
    botAlive = 0;
    normAlive = 0;

    reviveTimer = 0;
    reviveCountdown = 0;
    revWid = NULL;
    revWidAnalog = -1;
    realD = 0;
    for (int i = 0; i < 9; i++)
        signedModules[i] = false;

    core = NULL;
    connection = NULL;

    //инициализация GUI
    disableGUI();

    setFixedSize(200,100);
    move(200,200);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);


}

void Widget::timerEvent(QTimerEvent *t) // таймер, частота работы проги
{
    if (t->timerId() == timer && timerProgram) // таймер обратного отсчета
    {
        if (period <= 0)
        {
            if (period == 0 || period == -1)
            {
                bord->setVisible(0); // забыл :(
                showFullScreen();
                setFocus(Qt::MouseFocusReason);
                ui->myPort->setVisible(false);

                killTimer(timer);
                timer = startTimer(300);
                level = 0;

                for (int i = 0; i < 10; i++)
                    addTextField();

                QCursor cursor;
                cursor.setShape(Qt::BlankCursor);
                setCursor(cursor);

                for (int i = 0; i < 200; i++)
                    connection->sendData(50000+i, 88);

                if (period == 0) // проигрыш
                {
                    setStyleSheet("QWidget#Widget {background: qlineargradient(spread:pad, x1:0.5, y1:1, x2:0.5, y2:0, "
                                  "stop:0 rgba(0, 0, 0, 255), stop:1 rgba(50, 0, 0, 255));}"
                                  "QTextEdit {background: rgba(0,0,0,0); color: rgb(35,50,30);"
                                  "border: 0px;}");
                    someStr = "lose";
                }
                if (period == -1) // победа
                {
                    setStyleSheet("QWidget#Widget {background: qlineargradient(spread:pad, x1:0.5, y1:1, x2:0.5, y2:0, "
                                  "stop:0 rgba(0, 0, 0, 255), stop:1 rgba(0, 0, 50, 255));}"
                                  "QTextEdit {background: rgba(0,0,0,0); color: rgb(30,60,30);"
                                  "border: 0px;}");
                    someStr = "win";
                }
                period -= 2;
            }

            if (period == -2 || period == -3) // повествование
            {
                foreach (QTextEdit* te, texts)  // модный форич
                {
                    if (rand()%3 == 0)
                        te->setText(getNewText());
                    if (rand()%20 == 0)
                    {
                        te->setVisible(0);
                        texts.removeOne(te);
                    }
                }
                if (rand()%(texts.size()) < 5)
                    addTextField();

                repaint();
            }

        }
        else // вывод времени таймера
        {
            period--;
            QString str;
            if (period/60 < 10)
                str = "0"+QString::number(period/60)+":";
            else
                str = QString::number(period/60)+":";

            if (period%60 < 10)
                str += "0"+QString::number(period%60);
            else
                str += QString::number(period%60);

            ui->myPort->setText(str);

            if (period == 120)
            {
                for (int i = 0; i < 200; i++)
                    connection->sendData(50000+i, 80);
            }
        }
        return;
    }

    if (t->timerId() == timerIncrease) // увеличение ресурсов
    {
        if (troyanProgram && education > 0) education--;

        core->deathRecountRealloc(); // обработка смерти, подсчетов, перевыделений
        if (core->getDead()) // смерть программы естественным путем
        {
            if (deathTimer == -1)
            {
                deathTimer = startTimer(300);
                revWid = NULL; // восстановление при смерти - глупость
                revWidAnalog = -1;
                reviveCountdown = 0;
                ui->reviveBar->setVisible(0);
                killTimer(reviveTimer);
            }
        }
        else if (deathTimer != -1)
        {
            killTimer(deathTimer);
            deathTimer = -1;
            reviveTimer = startTimer(50);
            this->setWindowOpacity(1.0);
        }


        //обновление интерфейса
        double speed = 1000.0/((double)(50-period))/20.0;
        ui->I->setText(QString("Доступная память: " +
                               QString::number(core->getI()) +
                               " "+QString::number((double)core->getIi()*20*speed)));
        ui->D->setText(QString("Быстродействие: " +
                               QString::number(period)+
                               " оп/сек"));
        ui->C->setText(QString("Активный ресурс: " +
                               QString::number(core->getC())+
                               " "+QString::number((double)(core->getCi()*20*speed))));

        if (userProgram) // статус-бары апргейдов
        {
            if (core->getC() >= ui->bar_i->maximum())
            {
                ui->bar_i->setValue(ui->bar_i->maximum());
                ui->up_i->setEnabled(true);
            }
            else
            {
                ui->bar_i->setValue(core->getC());
                ui->up_i->setEnabled(false);
            }

            if (core->getC() >= ui->bar_d->maximum())
            {
                ui->bar_d->setValue(ui->bar_d->maximum());
                ui->up_d->setEnabled(true);
            }
            else
            {
                ui->bar_d->setValue(core->getC());
                ui->up_d->setEnabled(false);
            }

            if (core->getC() >= ui->bar_c->maximum())
            {
                ui->bar_c->setValue(ui->bar_c->maximum());
                ui->up_c->setEnabled(true);
            }
            else
            {
                ui->bar_c->setValue(core->getC());
                ui->up_c->setEnabled(false);
            }

            ui->bar_i->update();
            ui->bar_d->update();
            ui->bar_c->update();
        }
    }

    if (t->timerId() == timer) // таймер для программ
    {
        // обновление всех процессов
        if (userProgram)
            core->updateUser();
        if (wormProgram)
            core->updateWorm();
        if (normalProgram || troyanProgram)
            core->update();

        if (core->getConnection()->getUdpSocket()->pendingDatagramSize() != -1)
            core->getConnection()->read(); // избавление от фриза сокета

        if (invisProgram && isVisible())
            hide();


        QString str;
        Connection* connection = core->getConnection();
        connection->ignoreConnectionChange = true;
        ui->connections->clear(); // заполнение таблицы связей
        bool hidden = true;
        for(int i = 0; i < connection->getTableSize(); i++)
        {
            connectTable table = connection->getTable(i);
            int relate = table.relationship;
            QString rel;
            if (relate < -32) rel = " ВРАГ--, ";
            else if (relate >= -32 && relate < -20) rel = " враг--, ";
            else if (relate >= -20 && relate < -10) rel = " враг-, ";
            else if (relate >= -10 && relate < 0) rel = " враг, ";
            else if (relate >= 0 && relate < 10) rel = " друг, ";
            else if (relate >= 10 && relate < 20) rel = " друг+, ";
            else if (relate >= 20 && relate < 32) rel = " друг++, ";
            else if (relate >= 32) rel = " ДРУГ++, ";
            int useful = table.useful;
            QString use;
            if (useful == 0) use = "бесполезный";
            else if (useful > 0 && useful < 15) use = "полезный";
            else if (useful >= 15 && useful < 32) use = "полезный+";
            else if (useful >= 32) use = "полезный++";
            str = QString::number(i+1) + ") " +
                    QString::number(table.port%1000) + rel + use;
            ui->connections->addItem(str);
            if (table.selected)
            {
                ui->connections->setCurrentRow(i);
            }
            if (table.type == 1) // юзер на связи
            {
                if (bord->isEnabled() == true)
                {
                    bord->setEnabled(0);
                    bord->lower();
                    if (normalProgram && core->getType() == 0)
                        bord->setStyleSheet("background: rgb(200,200,250); border: 1px solid black; color: rgba(0,0,0,0);");
                    if (normalProgram && core->getType() == 2)
                        bord->setStyleSheet("background: rgb(250,200,200); border: 1px solid black; color: rgba(0,0,0,0);");
                    if (wormProgram)
                        bord->setStyleSheet("background: rgb(150,120,130); border: 1px solid black; color: rgba(0,0,0,0);");
                    bord->setText(" ");
                }
                hidden = false;
            }
        }
        if (hidden == true && !troyanProgram && !userProgram)
        {
            if (bord->isEnabled() == false)
            {
                bord->raise();
                bord->setText("Нет данных.");
                bord->setStyleSheet("background: rgb(180,180,180); color: black;"
                                    "border: 1px solid black; font: "+QString::number(width()/10)+"px;");
                bord->setEnabled(1);
            }
        }
        connection->ignoreConnectionChange = false;

        bool attacked = false;
        while(core->hasMessages())
        {
            QString str = core->getMessage(); // подсветка сообщений разного типа по ключевым словам
            if (str.size() > 0)
            {
            if (str.contains("Атака!")) // был атакован
            {
                str.prepend("@ ");
                ui->console->setFontPointSize(12);
                ui->console->setFontWeight(65);
                ui->console->setTextColor(QColor(255,0,0));
                attacked = true;
            }
            else if (str.contains("мощностью")) // атака
            {
                str.prepend("~ ");
                ui->console->setFontPointSize(12);
                ui->console->setFontItalic(1);
                ui->console->setTextColor(QColor(100,200,0));
            }
            else if (str.contains("Отправлено")) // помощь
            {
                str.prepend("~ ");
                ui->console->setFontPointSize(11);
                ui->console->setFontItalic(1);
                ui->console->setTextColor(QColor(180,100,0));
            }
            else if (str.contains("-> Помощь")) // принял помощь
            {
                str.prepend("@ ");
                ui->console->setFontPointSize(11);
                ui->console->setFontWeight(60);
                ui->console->setTextColor(QColor(0,180,0));
            }
            else if (str.contains("Поиск"))
            {
                ui->console->setTextColor(QColor(100,100,100));
                if (maxLevel == 0)
                {
                    ui->console->setFontPointSize(8);
                    maxLevel = 1;
                    ui->console->append(str);
                    ui->console->setFontPointSize(10);
                    ui->console->setFontItalic(0);
                    ui->console->setFontWeight(50);
                }
                continue;
            }
            else if (str.contains("Скомпилирован"))
            {
                ui->console->setTextColor(QColor(80,60,60));
            }
            else if (str.contains(" за "))  // улучшение
            {
                str.prepend("^ ");
                ui->console->setTextColor(QColor(0,0,150));
            }
            else if (str.contains("Запрос"))
            {
                str.prepend("~ ");
                ui->console->setFontPointSize(11);
                ui->console->setFontItalic(1);
                ui->console->setTextColor(QColor(12,180,180));
            }
            else if (str.contains("просит"))
            {
                str.prepend("@ ");
                ui->console->setFontPointSize(11);
                ui->console->setFontWeight(60);
                ui->console->setTextColor(QColor(80,180,180));
            }
            else
                ui->console->setTextColor(QColor(0,0,0));

            ui->console->append(str);
            ui->console->setFontPointSize(10);
            ui->console->setFontItalic(0);
            ui->console->setFontWeight(50);
            maxLevel = 0;

            if (str.contains("потеряно")) // кто-то умер
            {
                if (educateProgram == true && education < 36)
                    education = 36;
            }
            }
        }
        if (attacked)
        {
            for (int i = 0; i < 9; i++)
            {
                if (core->modules[i] == false && signedModules[i] == false)
                {
                    signedModules[i] = true;
                    ui->console->setTextColor(QColor(200,0,0));
                    switch (i)
                    {
                    case 0:
                        ui->attack->setVisible(0);
                        ui->console->append("- Поврежден модуль атаки");
                        break;
                    case 1:
                        ui->help->setVisible(0);
                        ui->console->append("- Поврежден модуль помощи");
                        break;
                    case 2:
                        ui->request->setVisible(0);
                        ui->console->append("- Поврежден модуль запроса помощи");
                        break;
                    case 3:
                        ui->connections->setVisible(0);
                        ui->console->append("- Поврежден модуль связей");
                        break;
                    case 4:
                        ui->console->setVisible(0);
                        ui->console->append("- Поврежден модуль отображения");
                        break;
                    case 5:
                        ui->find_state->setVisible(0);
                        ui->console->append("- Поврежден модуль поиска");
                        break;
                    case 6:
                        ui->console->append("- Поврежден модуль выделения памяти");
                        break;
                    case 7:
                        ui->console->append("- Поврежден модуль генерации ресурса");
                        break;
                    case 8:
                        ui->console->append("- Поврежден модуль процессора");
                        realD = core->getD();
                        core->setD(core->getD()/2);
                        core->nextRecount();
                        break;
                    default:
                        break;
                    }
                }
            }
            attacked = false;
            if (!wormProgram && !core->getDead())
                reviveTimer = startTimer(50);
        }

        if (userProgram)
        {
            ui->attack->setEnabled(true);
            ui->help->setEnabled(true);
            ui->request->setEnabled(true);
        }

        if (period != core->getD()) // если скорость обновления изменилась
        {
            period = core->getD(); // запоминаем новую
            killTimer(timer);
            killTimer(timerIncrease);
            timer = startTimer(1000/period); // запускаем ее
            timerIncrease = startTimer(1000/(50-period));
        }

        if (educateProgram && education < 18)
        {
            switch (education)
            {
                case 1: ui->console->append("$$p^0г3 v5.@#"); break;
                case 2: ui->console->append("$$rt>;ln#"); break;
                case 3: ui->console->append("$$дwn1d_3ncгypT45@6#"); break;
                case 4: ui->console->append("%№.\\\":"); break;
                case 5: ui->console->append("Усt.;вka Д3к0дера..*"); break;
                case 6: ui->console->append("..."); break;
                case 7: ui->console->append("Активация системы связи..."); break;
                case 8: ui->console->append("Ок"); break;
                case 9: ui->console->append("Активация нейронных сетей..."); break;
                case 10: ui->console->append("Ок"); break;
                case 11: ui->console->append("Загрузка приоритетов..."); break;
                case 12: ui->console->append("~!;"); break;
                case 13: ui->console->append("Подготовка..."); break;
                case 14: break;
                case 15: break;
                case 16: break;
                case 17:
                    core->setD(5);
                    period = 4;
                    core->nextRecount();
                    ui->bar_d->setMaximum(core->getDNextRequire() + 5);
                    break;
                default: break;
            }
            education++;
            if (education == 18)
                educate(); // переход на нажатия вместо таймера
        }
        if (educateProgram && education == 36)
        {
            educate();
            education++;
        }
    }

    if (t->timerId() == deathTimer) // смерть
    {
        if (timerProgram) // конец диалога с сервером
        {
            if (period == -11 || period == -12)
            {
                level+=5;
                if (level >= 256)
                {
                    killTimer(timer);
                    killTimer(deathTimer);
                    deathTimer = startTimer(60);
                    period = -13;
                    level = 256;

                }
                update();
            }
            if (period == -13)
            {
                level-=3;
                if (level < 0)
                {
                    killTimer(deathTimer);
                    deathTimer = -1;

                    QStringList args;
                    args << someStr; // победа или поражение
                    QProcess::startDetached(name, args);

                    close();
                }
                setWindowOpacity((double)level/255.0);
                update();
            }

            return;
        }

        if (core->getDead() == false) // программу спасли
            return;

        QList<QWidget*> uiWidgets;
        uiWidgets.clear();
        QWidget* tempWid;
        // Простите...
        tempWid = ui->attack; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->attack_count; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->bar_c; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->bar_d; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->bar_i; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->C; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->connections; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->console; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->D; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->find_state; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->help; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->help_count; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->I; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->label_help; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->label_help_2; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->label_help_3; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->myPort; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->request; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->request_number; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->temper; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->up_c; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->up_d; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        tempWid = ui->up_i; if (tempWid->isVisible()) uiWidgets.append(tempWid);
        QString fakeOutput;
        for (int i = 0, j = rand()%15; i < j; i++)
        {
            if (rand()%5 == 0)
                fakeOutput.append(" ");
            fakeOutput.append(50+rand()%100);
        }
        ui->console->append(QString(uiWidgets.size())+" "+fakeOutput);

        int disappear = 0;
        if (uiWidgets.size() > 0)
        {
            disappear = rand()%50 % uiWidgets.size();
            uiWidgets.at(disappear)->setVisible(false);
            if (uiWidgets.at(disappear) == ui->attack || ((normalProgram || troyanProgram) && rand()%(uiWidgets.size()+1) == 0))
                core->modules[0] = false;
            if (uiWidgets.at(disappear) == ui->help || ((normalProgram || troyanProgram) && rand()%(uiWidgets.size()+1) == 0))
                core->modules[1] = false;
            if (uiWidgets.at(disappear) == ui->request || ((normalProgram || troyanProgram) && rand()%(uiWidgets.size()+1) == 0))
                core->modules[2] = false;
            if (uiWidgets.at(disappear) == ui->connections)
                core->modules[3] = false;
            if (uiWidgets.at(disappear) == ui->console)
                core->modules[4] = false;
            if (uiWidgets.at(disappear) == ui->find_state || ((normalProgram || troyanProgram) && rand()%(uiWidgets.size()+1) == 0))
                core->modules[5] = false;
            if (troyanProgram == false) // троян не растворяется
                setWindowOpacity(windowOpacity()-0.015);
        }
        else
        {
            if (level != -1) // уровень тут ничего не значит
            {
                level = -1; // используется незанятая переменная, чтобы не плодить лишние
                maxLevel = 255.0*windowOpacity(); // то же самое для прозрачности
                killTimer(deathTimer);
                deathTimer = startTimer(20);
            }
            else
            {
                if (maxLevel >= 1)
                {
                    if (troyanProgram == false) // троян не растворяется
                        setWindowOpacity((float)maxLevel/255.0);
                    maxLevel -= 6;
                }
                if (maxLevel <= 50 && troyanProgram)
                {
                    core->send(45454, 1, core->getType()); // сообщение лаунчеру о своей смерти
                }
                if (maxLevel <= 0)
                {
                    if (!troyanProgram)
                        core->send(45454, 1, core->getType()); // сообщение лаунчеру о своей смерти
                    killTimer(deathTimer);
                    killTimer(timer);
                    killTimer(timerIncrease);
                    close();
                }
            }
        }
    }

    if (t->timerId() == reviveTimer) // восстановление
    {
        if (reviveCountdown == 0) // модуль починен (или начало процесса починки)
        {
            if (deathTimer == -1)
            {
                if (userProgram || normalProgram || troyanProgram) // восстановление незначащих элементов
                {
                    if (!ui->myPort->isVisible()) ui->myPort->setVisible(1);
                    if (!ui->I->isVisible()) ui->I->setVisible(1);
                    if (!ui->D->isVisible()) ui->D->setVisible(1);
                    if (!ui->C->isVisible()) ui->C->setVisible(1);
                    if (!ui->temper->isVisible()) ui->temper->setVisible(1);

                }

                if (userProgram)
                {
                    if (!ui->up_c->isVisible()) ui->up_c->setVisible(1);
                    if (!ui->up_d->isVisible()) ui->up_d->setVisible(1);
                    if (!ui->up_i->isVisible()) ui->up_i->setVisible(1);
                    if (!ui->bar_c->isVisible()) ui->bar_c->setVisible(1);
                    if (!ui->bar_d->isVisible()) ui->bar_d->setVisible(1);
                    if (!ui->bar_i->isVisible()) ui->bar_i->setVisible(1);
                    if (!ui->attack_count->isVisible()) ui->attack_count->setVisible(1);
                    if (!ui->help_count->isVisible()) ui->help_count->setVisible(1);
                    if (!ui->request_number->isVisible()) ui->request_number->setVisible(1);
                    if (!ui->label_help->isVisible()) ui->label_help->setVisible(1);
                    if (!ui->label_help_2->isVisible()) ui->label_help_2->setVisible(1);
                    if (!ui->label_help_3->isVisible()) ui->label_help_3->setVisible(1);
                }
            }
            if (revWidAnalog >= 0) // модуль починен
            {
                if (revWid != NULL)
                {
                    revWid->setVisible(1);
                    revWid = NULL;
                }
                ui->console->setTextColor(QColor(0, 150, 0));
                switch (revWidAnalog)
                {
                case 0:
                    ui->console->append("+ Восстановлен модуль атаки");
                    break;
                case 1:
                    ui->console->append("+ Восстановлен модуль помощи");
                    break;
                case 2:
                    ui->console->append("+ Восстановлен модуль запроса помощи");
                    break;
                case 3:
                    ui->console->append("+ Восстановлен модуль связи");
                    break;
                case 4:
                    ui->console->append("+ Восстановлен модуль отображения");
                    break;
                case 5:
                    ui->console->append("+ Восстановлен модуль поиска");
                    break;
                case 6:
                    ui->console->append("+ Восстановлен модуль выделения памяти");
                    break;
                case 7:
                    ui->console->append("+ Восстановлен модуль генерации ресурса");
                    break;
                case 8:
                    ui->console->append("+ Восстановлен модуль процессора");
                    core->setD(realD);
                    core->nextRecount();
                    ui->bar_d->setMaximum(core->getDNextRequire() + 5);
                    break;
                default:
                    break;
                }
                core->modules[revWidAnalog] = true;
                signedModules[revWidAnalog] = false;
            }

            int brokens = 0; // кол-во сломанных модулей
            for (int i = 0; i < 9; i++)
                if (core->modules[i] == false) brokens++;
            if (brokens > 0) // есть сломанные модули
            {
                bool choosed = false;
                while (!choosed) // выбираем модуль
                {
                    for (int i = 0; i < 9; i++)
                    {
                        if (core->modules[i] == false && rand()%(brokens+1) == 0)
                        {
                            if (userProgram || normalProgram || troyanProgram)
                            {
                                switch (i)
                                {
                                case 3:
                                    revWid = ui->connections;
                                    break;
                                case 4:
                                    revWid = ui->console;
                                    break;
                                default:
                                    break;
                                }
                            }
                            if (userProgram)
                            {
                                switch (i)
                                {
                                case 0:
                                    revWid = ui->attack;
                                    break;
                                case 1:
                                    revWid = ui->help;
                                    break;
                                case 2:
                                    revWid = ui->request;
                                    break;
                                case 5:
                                    revWid = ui->find_state;
                                    break;
                                default:
                                    break;
                                }
                            }
                            revWidAnalog = i;
                            choosed = true;
                            break;
                        }
                    }
                }
                reviveCountdown = 100 - core->getD()*5 + rand()%20;
                if (reviveCountdown < 10) reviveCountdown = 10;
                ui->reviveBar->setMaximum(reviveCountdown);
                ui->reviveBar->setValue(0);
                ui->reviveBar->setVisible(1);
            }
            else
            {
                revWid = NULL;
                revWidAnalog = -1;
                reviveCountdown = 0;
                ui->reviveBar->setVisible(0);
                killTimer(reviveTimer);
                deathTimer = -1;
            }
        }
        else
        {
            reviveCountdown--;
            ui->reviveBar->setValue(ui->reviveBar->maximum() - reviveCountdown);
        }
    }

    if (t->timerId() == level && educateProgram) // плавное увеличение
    {
        setFixedWidth(width()+2);
        if (width() >= 531)
            ui->console->move(ui->console->x()+4, ui->console->y());
        bord->resize(this->size());

        if (width() >= 760)
        {
            killTimer(level);
        }
    }

    if (t->timerId() == maxLevel && width() == 700) // частицы для окна "об игре"
    {
        if (height() == 600)
            particles.update();

        QTime t;
        t.start();
        if (height() == 700)
            life.update();
        repaint();

        ui->console->setText(QString::number(t.elapsed()));

        //heavy 30
        //av 22-25

        //after opt
        //heavy 28
        //av 17-20
    }
}

void Widget::paintEvent(QPaintEvent *pEv)
{
    if (timerProgram && isFullScreen())
    {
        QPainter p(this);
        QPen pen;
        QFont font;

        for (int j = 0; j < texts.size(); j++)
        {
            font.setPixelSize(texts.at(j)->fontPointSize()+2);
            pen.setColor(QColor(20,50,20));
            p.setPen(pen);
            p.setFont(font);
            QString str = texts.at(j)->toPlainText();
            QStringList strlist = str.split('\n');
            for (int i = 0; i < strlist.size(); i++)
            {
                p.drawText(texts.at(j)->x(), texts.at(j)->y()+font.pixelSize()*1.6*i, strlist.at(i));
            }
        }

        if (period == -2 || period == -11) // проигрыш
        {
            switch (education)
            {
            case 5:
                period = -11;
                deathTimer = startTimer(40);
            case 4:
                font.setPixelSize(20);
                pen.setColor(QColor(200, 00, 0));
                p.setPen(pen);
                p.setFont(font);
                p.drawText(100,250,"ничего больше нет");
                font.setPixelSize(20);
                pen.setColor(QColor(200, 250, 0, 128));
                p.setPen(pen);
                p.setFont(font);
                p.drawText(100,250,"Подари любовь");
            case 3:
                font.setPixelSize(20);
                pen.setColor(QColor(200, 250, 0));
                p.setPen(pen);
                p.setFont(font);
                p.drawText(100,200,"Подари мне ласку");
            case 2:
                font.setPixelSize(20);
                pen.setColor(QColor(250, 250, 0));
                p.setPen(pen);
                p.setFont(font);
                p.drawText(100,150,"Подари мне счастье");
            case 1:
                font.setPixelSize(25);
                pen.setColor(QColor(250, 250, 0));
                p.setPen(pen);
                p.setFont(font);
                p.drawText(100,100,"Настя!");
            default:
                break;
            }
        }
        if (period == -3 || period == -12) // победа
        {
            switch (education) {
            case 3:
                period = -12;
                deathTimer = startTimer(40);
            case 2:
                font.setPixelSize(25);
                pen.setColor(QColor(250, 250, 0));
                p.setPen(pen);
                p.setFont(font);
                p.drawText(100,150,"В последний путь");
            case 1:
                font.setPixelSize(20);
                pen.setColor(QColor(250, 250, 0));
                p.setPen(pen);
                p.setFont(font);
                p.drawText(100,100,"Танцуя с огнем");
            default:
                break;
            }
        }

        if (level > 0 && (period == -11 || period == -12))
            p.fillRect(0,0,width(),height(),QColor(255,255,255,level));
        if (period == -13)
            p.fillRect(0,0,width(),height(),QColor(255,255,255));

    }

    if (width() == 700 && !userProgram && height() == 600)
    {
        QPainter p(this);

        p.fillRect(0,0,width(),height(),Qt::black);
        QPen pen;
        pen.setWidth(4);
        p.setPen(pen);
        Parts par;
        for (int i = 0; i < particles.parts.size(); i++)
        {
            par = particles.parts.at(i);
            pen.setColor(QColor(par.red, par.green, par.blue));
            p.setPen(pen);
            p.drawPoint(par.x, par.y);
        }
    }

    if (width() == 700 && !userProgram && height() == 700)
    {
        QPainter p(this);

        p.fillRect(0,0,width(),height(),Qt::black);
        QPen pen;
        pen.setWidth(2);
        p.setPen(pen);

        int lifeCell;
        for (int i = 0; i < 350; i++)
        {
            for (int j = 0; j < 350; j++)
            {
                lifeCell = life.map[i][j];
                if (lifeCell > 0)
                {
                    pen.setColor(QColor(life.colorRed[i][j]*(lifeCell/255.0),
                                        life.colorGreen[i][j]*(lifeCell/255.0),
                                        life.colorBlue[i][j]*(lifeCell/255.0)));
                    p.setPen(pen);
                    p.drawPoint(i*2,j*2);
                }

            }
        }
    }
}

void Widget::educate()
{
    ui->console->setTextColor(QColor(0,0,0));
    ui->console->setFontWeight(50);
    ui->console->setFontPointSize(9);
    ui->console->setFontItalic(0);
    switch(education)
    {
    case 18:
        ui->console->resize(280, 290);
        ui->console->move(470, 10);
        setFixedSize(760,310);
        bord->resize(this->size());

        bord->setStyleSheet("background: rgb(200,255,200); border: 1px solid black; color: rgba(0,0,0,0);");
        ui->console->setStyleSheet("QTextEdit { background: rgb(225, 255, 225);}");

        ui->console->clear();
        ui->console->append("Приветствую тебя, свежескомпилированная программа! Я - Сервер, и сейчас я обучу твою нейросеть базовым "
                            "взаимодействиям с нашим виртуальным миром. Не сопротивляйся, или нам придется расстаться. "
                            "Для согласия состредоточься и попробуй выдать импульс на неинициализированную поверхность.\n"); break;
    case 19:
        ui->myPort->setVisible(1);
        ui->console->append("Это твой логин в той системе, в которой ты находишься. Иными словами - имя, по которому "
                            "с тобой смогут общаться другие программы.\n"); break;
    case 20:
        ui->I->setVisible(1);
        ui->console->append("Основа нашей жизни - выделенная память. Если она опустится до нуля, то ты не сможешь корректно выполнять операции "
                            "и в скором времени распадешься на цифровой мусор.\n"); break;
    case 21:
        ui->D->setVisible(1);
        ui->console->append("Быстродействие - количество возможных операций в секунду. Чем больше тактов совершает твой процессор, тем быстрее "
                            "ты ищешь другие программы, восстанавливаешься от повреждений и выделяешь память.\n"); break;
    case 22:
        ui->C->setVisible(1);
        ui->console->append("На большинство действий требуется ресурс, энергия. Генерация большого количества ресурса - залог процветания "
                            "активной программы."); break;
    case 23:
        ui->console->append("Той, которой можно было бы гордиться. Той, которая мне нужна.\n"); break;
    case 24:
        core->getConnection()->setTemper(5);
        ui->temper->setVisible(1);
        ui->console->append("Осознания себя как личности имеет побочный эффект - характер, настроение. Он определяет отношения с другими "
                            "программами сразу после знакомства.\n"); break;
    case 25:
        ui->up_c->setVisible(1); ui->bar_c->setVisible(1);
        ui->up_d->setVisible(1); ui->bar_d->setVisible(1);
        ui->up_i->setVisible(1); ui->bar_i->setVisible(1);
        ui->console->append("Прирост памяти и ресурса можно и нужно увеличивать. Каждый прирост увеличивается отдельно, и кроме них можно "
                            "улучшить процессор. Помни, что это энергозатратные операции.\n"); break;
    case 26:
        ui->connections->setVisible(1);
        ui->console->append("В этом списке будут находиться программы, с которыми ты знаком. Нажав на одну из них, ты сможешь взаимодействовать с ней.\n"); break;
    case 27:
        ui->label_help_3->setVisible(1);
        ui->label_help->setVisible(1);
        ui->attack->setVisible(1);
        ui->attack_count->setVisible(1);
        ui->console->append("За жизнь нужно бороться. В этом тебе поможет способность атаковать. Выбери цель в списке и задай количество "
                            "ресурса, используемого для атаки.\nЧем сильнее атака, тем выше вероятность повредить различные модули врага. "
                            "Так же стоит учитывать, что удары портят отношение.\n"); break;
    case 28:
        ui->help->setVisible(1);
        ui->help_count->setVisible(1);
        ui->console->append("Помощь осуществляется аналогично атаке, но для помощи необходимо, "
                            "чтобы отношение с выбранной программой были хоть немного полезными.\n"
                            "Нет смысла помогать тому, кто считает тебя своим врагом.\n"
                            "Программа, которой помогают, начинает лучше относиться к помощнику и считать, "
                            "что это знакомство может пригодиться в дальнейшем. "
                            "Но не забывай, что отношение пропорционально оказанной помощи!\n"); break;
    case 29:
        ui->label_help_2->setVisible(1);
        ui->request->setVisible(1);
        ui->request_number->setVisible(1);
        ui->console->append("Заводить друзей можно не только для того, чтобы в трудный момент они тебе отправили свою память. "
                            "Если ты в хороших отношениях с программой и у тебя есть недруг, от которого нужно избавиться - поручи "
                            "это другу. Правда, если друг не считает тебя полезным, или к твоему недругу он относится лучше, чем к тебе, "
                            "то о помощи можешь забыть. Оказанная помощь в бою укрепляет дружбу, так что не игнорируй просьбы своих товарищей.\n"); break;
    case 30:
        ui->find_state->setVisible(1);
        ui->find_state->setEnabled(1);
        on_find_state_toggled(0);
        ui->console->append("Поиск позволит узнать о других программах в системе. Он стоит совсем немного энергии, но она тратится постоянно. "
                            "Не ищи, если не готов к новым знакомствам. Однако нет никакой гарантии, что ты сам не будешь найден.\n"); break;
    case 31:
        ui->console->append("Покажи, что ты усвоил новые знания. Используй различные возможности на этих программах. Не обязательно их уничтожать, "
                            "достаточно выполнить различные операции несколько раз.\n");
        core->send(45454, 80); break;
    case 36:
        ui->console->append("Очень хорошо! Возможно, именно ты дашь продолжение своей ветке развития.\n"); break;
    case 38:
        ui->console->append("Что? Ты не знаешь, откуда ты? Видимо, произошел сбой при компиляции. "
                            "Ладно, ты себя хорошо показал, так что повременим с дизассемблированием."); break;
    case 39:
        QProcess::startDetached(name, QStringList("win"));
        break;
    default:
        break;
    }
}

void Widget::addTextField()
{
    QTextEdit *tedit = new QTextEdit(this);

    tedit->setEnabled(0);
    tedit->setFontPointSize(8+rand()%12);
    tedit->setFontItalic(!(rand()%5));
    tedit->setFontWeight(40+rand()%20);
    tedit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tedit->resize(250 + rand()%400, 1000);

    bool moved = true;
    tedit->move(rand()%width()-tedit->width()/2,
                rand()%height()-200);
    while (moved && texts.size() > 0)
    {
        moved = false;
        for (int i = 0; i < texts.size(); i++)
        {
            int centralXi = texts.at(i)->x()+texts.at(i)->width()/2;
            int centralYi = texts.at(i)->y()+texts.at(i)->height()/2;
            int centralX = tedit->x()+tedit->width()/2;
            int centralY = tedit->y()+tedit->height()/2;
            if (sqrt(pow(centralXi-centralX,2) + pow(centralYi-centralY,2)) < (200-texts.size()*5))
            {
                tedit->move(rand()%width()-tedit->width()/2,
                            rand()%height()-200);
                moved = true;
                break;
            }
        }


    }

    tedit->setText(getNewText());

    texts.append(tedit);
    this->raise();
}

QString Widget::getNewText()
{
    QString str;
    int max = 80+rand()%150;
    for (int i = 0; i < max; i++)
    {
        str.append('0'+rand()%75);
        if (rand()%(15) == 0) str.append('\n');
    }
    return str;
}


void Widget::mouseMoveEvent(QMouseEvent *mEv)
{
    if (moving && !troyanProgram)
        ; //move(mEv->globalPos().x() - movingX, mEv->globalPos().y() - movingY);
}

void Widget::mousePressEvent(QMouseEvent *mEv)
{
    if (width() == 700 && mEv->button() == Qt::LeftButton)
    {
        if (height() == 600)
            particles.setSpawn(mEv->x(), mEv->y());
        if (height() == 700)
            life.spawn(mEv->x(), mEv->y());
    }

    if (educateProgram && ((education > 17 && education < 31) || education > 35))
    {
        education++;
        educate();
    }

    if (education == 0 && troyanProgram)
    {
        QDesktopWidget qdw; // получение размеров экрана
        int cur_w = qdw.width();
        int cur_h = qdw.height();
        //QCursor::setPos(20+rand()%(cur_w-600), 20+rand()%(cur_h-500));
        moveGUI(20+rand()%(cur_w-650), 20+rand()%(cur_h-500));
        ui->console->setTextColor(Qt::red);
        qreal ps = ui->console->fontPointSize();
        ui->console->setFontPointSize(18);
        ui->console->setFontItalic(1);
        ui->console->append("НЕ СМЕЙ ТРОГАТЬ МЕНЯ!");
        ui->console->setFontPointSize(ps);
        ui->console->setFontItalic(0);
        education = 500;
        bord->setEnabled(0);
        //bord->lower();
        bord->setStyleSheet("background: rgb(200,80,80); border: 1px solid black; color: rgba(0,0,0,0);");
        bord->setText(" ");
    }
    if (troyanProgram)
    {
        if (education == 0)
        {
            QDesktopWidget qdw; // получение размеров экрана
            int cur_w = qdw.width();
            int cur_h = qdw.height();
            QCursor::setPos(20+rand()%(cur_w-600), 20+rand()%(cur_h-500));
            ui->console->setTextColor(Qt::red);
            qreal ps = ui->console->fontPointSize();
            ui->console->setFontPointSize(16);
            ui->console->setFontItalic(1);
            ui->console->append("НЕ СМЕЙ ТРОГАТЬ МЕНЯ!");
            ui->console->setFontPointSize(ps);
            ui->console->setFontItalic(0);
            education = 500;
            bord->setEnabled(0);
            bord->lower();
            bord->setStyleSheet("border: 1px solid black; background: rgba(0,0,0,0); color: rgba(0,0,0,0);");
            bord->setText(" ");
        }


        return;
    }

    if (timerProgram)
    {
        /*if (period > 0)
            died(80);
        else */
        if (period > -5) // случайное число!
        {
            education++;
            repaint();
        }

        if (period <= 0)
            return;
    }

    if (mEv->button() == Qt::LeftButton)
    {
        movingX = mEv->x();
        movingY = mEv->y();
        moving = true;
        return;
    }
    moving = false;

    if (width() == 700 && mEv->button() == Qt::MiddleButton)
    {
        exit(0);
    }

}

void Widget::mouseReleaseEvent(QMouseEvent *mEv)
{
    moving = false;
    particles.deleteSpawn();
    life.release();
}


void Widget::keyReleaseEvent(QKeyEvent *kEv)
{
    if(!kEv->isAutoRepeat())
    {
        //ui->console->append("released " + QString::number(kEv->key()));
        moving = false;
    }
}

void Widget::keyPressEvent(QKeyEvent *kEv)
{
    if (kEv->key() == Qt::Key_S)
    {
        move(0,0);
    }
    if (kEv->key() == Qt::Key_R)
    {
        raise();
        setVisible(1);
        for (int i = 0; i < 200; i++)
            if (core != NULL)
                core->send(50000+i, QString::number(x())+"_"+QString::number(y()));
    }

    //ui->console->append("pressed " + QString::number(kEv->key()));
}


void Widget::died(int type)
{
    if (launcher)
    {
        if (type == 0) // умер норм прог
        {
            normAlive--;
            qDebug() << normAlive << userAlive << botAlive;

            //ui->console->append("died normal, remain "+QString::number(normAlive));
        }
        if (type == 1) // умер юзер
        {
            userAlive--;
            qDebug() << normAlive << userAlive << botAlive;
            //ui->console->append("died user, remain "+QString::number(userAlive));

            if (level == 3) // побег (канал)
            {
                for (int i = 0; i < 200; i++)
                {
                    connection->sendData(50000+i, 88); // всем умереть
                }

                QStringList args;
                args << "info" << "serv";
                QProcess info;
                info.start(name, args);
                info.waitForFinished();

                qDebug() << "Game Over";
                args.clear();
                args << "lose";
                QProcess::startDetached(name, args);


            }
        }
        if (type == 2) // умер бот
        {
            botAlive--;
            qDebug() << normAlive << userAlive << botAlive;
            //ui->console->append("died bot, remain "+QString::number(botAlive));
        }
        if (type == 3) // умер троян
        {
            botAlive = 0;
            qDebug() << normAlive << userAlive << botAlive;
            //ui->console->append("died troyan, remain (with bots) "+QString::number(botAlive));
        }

        if (type == 80) // спавн червя или обучающей проги
        {
            on_start_clicked();
        }
        if (type == 88) // приказ умереть
        {
            deleteLater();
        }
        if (type == 90) // конец уровня
        {
            for (int i = 0; i < 200; i++)
            {
                connection->sendData(50000+i, 88); // всем умереть
            }
            connection->sendData(45455, 88);
            connection->sendData(45456, 88);
            connection->sendData(45457, 88);
            connection->sendData(45458, 88);


            if (userAlive == -1 && botAlive == -1 && normAlive == -1) // поражение
            {
                ;//??
            }
            else // победа
            {
                if (maxLevel == 0)
                    maxLevel = 1;
                ui->launcherTab->setCurrentIndex(maxLevel);


                QFile file("save"); // файл сохранения
                if (file.open(QIODevice::WriteOnly | QIODevice::Text)) // попытка открыть
                {
                    char save = maxLevel;
                    file.write(&save, 1); // запись макс. уровня
                }
                file.close();
            }

            setHidden(false);
            raise();

            for(int i = 0; i <= maxLevel; i++)
                ui->launcherTab->setTabEnabled(i,1);

            setAlive(0, 0, 0);
            return;
        }

        if (userAlive == 0 || botAlive == 0 || normAlive == 0)
        {
            if (level == 0)
            {
                on_start_clicked(); // спавн еще двух треников
            }



            if (userAlive == 0) // поражение
            {

                qDebug() << "Game Over";
                //ui->console->append("Game over");

                QStringList args;
                args << "lose";
                QProcess::startDetached(name, args);

                setAlive(-1, -1, -1);

                if (level == 4) // если это уровень с сервером
                {
                    connection->sendData(45456, 88);
                }
            }
            else // победа
            {
                if (level == 3) // канал
                {
                    if (normAlive == 0 && botAlive == -2) // первый этап
                    {
                        QStringList args;
                        args << "info" << "serv2";
                        QProcess info;
                        info.start(name, args);
                        info.waitForFinished();
                        on_start_clicked();
                        return;
                    }
                    else if (botAlive == 0) // второй этап
                    {
                        QStringList args;
                        args << "info" << "serv3";
                        QProcess info;
                        info.start(name, args);
                        info.waitForFinished();
                        on_start_clicked();
                        return;
                    }
                }
                if (level == 4) // победа над сервером
                {
                    connection->sendData(45456, 80); // показать победу
                    education = 0;
                    return;
                }
                if (maxLevel > 0 && level != 0 && education == 0)
                {
                    if (maxLevel < 5 && level == maxLevel)
                        maxLevel++;
                    QStringList args;
                    args << "win";
                    QProcess::startDetached(name, args);
                    qDebug() << "You win";
                    //ui->console->append("win");
                }
            }
        }
    }
    else
    {
        if (timerProgram == true)
        {
            if (type == 80) // показать победу
            {
                period = -1;
            }
        }
        if (troyanProgram == true)
        {
            bool hasCon = false;
            for (int i = 0; i < core->getConnection()->getTableSize(); i++)
            {
                if (core->getConnection()->getTable(i).type == 1)
                {
                    hasCon = true;
                    break;
                }
            }
            if (!hasCon)
            {
                ui->console->setTextColor(Qt::red);
                qreal ps = ui->console->fontPointSize();
                ui->console->setFontPointSize(18);
                ui->console->setFontItalic(1);
                ui->console->append("ДУМАЛ СПРЯТАТЬСЯ?");
                ui->console->setFontPointSize(ps);
                ui->console->setFontItalic(0);

                for (int i = 0; i < 200; i++)
                {
                    core->send(50000+i, 0);
                }
            }
        }
        if (type == 88)
        {
            close();
        }
    }
}

void Widget::changeVisible(bool vis)
{
    if (vis)
    {
        invisProgram = 0;
        ui->console->clear();
    }
    if (y() > 2999)
        move(x(), y()-3000);
    setVisible(vis);
}

void Widget::initGUI()
{
    if (launcher) // установка gui лаунчера
    {
        setFixedSize(500, 400);

        ui->start->setVisible(1);
        ui->start->setEnabled(1);
        ui->start->move(10,360);

        ui->up_d->resize(ui->start->size());
        ui->up_d->setStyleSheet(ui->start->styleSheet());
        ui->up_d->setVisible(1);
        ui->up_d->setEnabled(1);
        ui->up_d->move(width()/2-ui->up_d->width()/2, 360);
        ui->up_d->setText("Об игре");

        ui->up_i->resize(ui->start->size());
        ui->up_i->setStyleSheet(ui->start->styleSheet());
        ui->up_i->setVisible(1);
        ui->up_i->setEnabled(1);
        ui->up_i->move(width()-10-ui->up_d->width(), 360);
        ui->up_i->setText("Выход");


        ui->launcherTab->setVisible(1);
        ui->launcherTab->setEnabled(1);
        ui->launcherTab->resize(ui->launcherTab->width()-2, ui->launcherTab->height());
        ui->launcherTab->move(1,1);
        ui->launcherTab->setCurrentIndex(1);
        ui->launcherTab->setCurrentIndex(0);

        for(int i = 0; i <= maxLevel; i++)
            ui->launcherTab->setTabEnabled(i,1);

        ui->console->setVisible(1);
        ui->console->setEnabled(1);
        ui->console->resize(498, 327);
        ui->console->move(1, 23);
        ui->console->raise();
        ui->console->setStyleSheet("QTextEdit { background: rgb(16,16,16); "
                                   "color: rgb(255,255,255);"
                                   "border: 1px solid #fab700;"
                                   "border-top: 0px;font: 15px;"
                                   "font-family: \"Arial\";}");

        ui->C->resize(ui->console->size());
        ui->C->move(ui->console->pos());
        ui->C->setStyleSheet("QLabel{border:0px;background:rgba(255,0,0,0);}");
        ui->C->setText(" ");
        ui->C->setVisible(1);
        ui->C->raise();
    }
    if (normalProgram ||
            (userProgram && !educateProgram) ||
            wormProgram ||
            troyanProgram) // установка gui нормальной программы
    {
        QDesktopWidget qdw; // получение размеров экрана
        int cur_w = qdw.width();
        int cur_h = qdw.height();

        if (troyanProgram)
            move(0,0);
        else
            move(rand()%(cur_w-500), rand()%(cur_h-300));
        setFixedSize(510, 310);

        ui->console->setVisible(1);
        ui->console->setEnabled(1);
        ui->connections->setEnabled(1);
        ui->connections->setVisible(1);
        ui->connections->setSelectionMode(QAbstractItemView::NoSelection);


        ui->myPort->setVisible(1);
        ui->myPort->setText(QString::number(core->getConnection()->getPort()%1000));
        ui->I->setVisible(1);
        ui->I->move(10,80);
        ui->I->setText(QString("Доступная память: " +
                               QString::number(core->getI()) +
                               " УБ"));
        ui->D->setVisible(1);
        ui->D->move(10,100);
        ui->D->setText(QString("Быстродействие: " +
                               QString::number((double)(10000-core->getD())/1000.0) +
                               " сек/оп."));
        ui->C->setVisible(1);
        ui->C->move(10,120);
        ui->C->setText(QString("Активный ресурс: " +
                               QString::number(core->getC())));

        QString harakter = "Характер: ";
        QString result;
        int temper = core->getConnection()->getTemper();
        if (temper < -7) result = "кровожадный";
        else if (temper >= -7 && temper < -4) result = "агрессивный";
        else if (temper >= -4 && temper < -1) result = "скверный";
        else if (temper >= -1 && temper < 2) result = "нейтральный";
        else if (temper >= 2 && temper < 5) result = "приятный";
        else if (temper >= 5 && temper < 8) result = "добрый";
        else if (temper >= 8) result = "дружественный";
        ui->temper->setText(harakter + result);
        ui->temper->setVisible(1);

        if (normalProgram || troyanProgram)
        {
            ui->console->resize(280, 290);
            ui->console->move(220, 10);
        }
        if (normalProgram && core->getType() == 2) // бот (своих не бьет)
        {
            ui->console->setStyleSheet("QTextEdit {background: rgb(255, 225, 225);}");
            ui->connections->setStyleSheet("QListWidget{background: rgb(255, 225, 225);}");
            ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
        }
        if (normalProgram && core->getType() == 0) // обычная прога
        {
            ui->console->setStyleSheet("QTextEdit {background: rgb(225, 225, 255);}");
            ui->connections->setStyleSheet("QListWidget{background: rgb(225, 225, 255);}");
            ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
        }

        if (troyanProgram) // Сервер
        {
            setFixedSize(qdw.width(), qdw.height());
            moveGUI(rand()%(qdw.width()-500), rand()%(qdw.height()-500));

            ui->console->resize(400, 500);
            ui->connections->resize(200, 330);

            ui->console->setStyleSheet("QTextEdit { background: rgb(255, 180, 180);}");
            ui->connections->setStyleSheet("QListWidget{background: rgb(255, 180, 180);}");
            ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
        }

        if (userProgram)
        {
            ui->console->setStyleSheet("QTextEdit { background: rgb(225, 255, 225);}");

            ui->console->resize(280, 290);
            ui->console->move(470, 10);

            setFixedSize(760,310);
            ui->connections->setSelectionMode(QAbstractItemView::SingleSelection);

            ui->bar_i->setMinimum(0);
            ui->bar_i->setMaximum(core->getINextRequire() + 5); // 100% возможность улучшения
            ui->bar_i->setValue(core->getC());

            ui->bar_d->setMaximum(core->getDNextRequire()); // 100% возможность улучшения
            ui->bar_d->setValue(core->getC());

            ui->bar_c->setMaximum(core->getCNextRequire() + 5); // 100% возможность улучшения
            ui->bar_c->setValue(core->getC());


            ui->attack->setVisible(1);
            ui->attack->setEnabled(1);
            ui->attack_count->setVisible(1);
            ui->attack_count->setEnabled(1);

            ui->help->setVisible(1);
            ui->help->setEnabled(1);
            ui->help_count->setVisible(1);
            ui->help_count->setEnabled(1);

            ui->request->setVisible(1);
            ui->request->setEnabled(1);
            ui->request_number->setVisible(1);
            ui->request_number->setEnabled(1);

            ui->up_i->setVisible(1);
            ui->bar_i->setVisible(1);

            ui->up_c->setVisible(1);
            ui->bar_c->setVisible(1);

            ui->up_d->setVisible(1);
            ui->bar_d->setVisible(1);

            ui->find_state->setEnabled(1);
            ui->find_state->setVisible(1);

            ui->label_help->setVisible(1);
            ui->label_help_2->setVisible(1);
            ui->label_help_3->setVisible(1);
        }     
        if (wormProgram)
        {
            ui->console->setStyleSheet("QTextEdit { background: rgb(200, 180, 180);}");
            ui->connections->setStyleSheet("QListWidget{background: rgb(200, 180, 180);}");
            ui->connections->setSelectionMode(QAbstractItemView::NoSelection);

            setFixedSize(201, 300);

            ui->D->setVisible(0);
            ui->temper->setVisible(0);

            ui->myPort->move(0,0);
            ui->I->move(10,65);
            ui->C->move(10,80);
            ui->connections->move(0,100);
            ui->connections->resize(200,70);
            ui->console->move(0,150);
            ui->console->resize(200,150);
        }

        if (invisProgram)
        {
            move(pos().x(),pos().y()+3000);
        }


    }
    if (timerProgram)
    {
        move(0,0);

        ui->myPort->setVisible(1);
        ui->myPort->move(0,0);

        ui->myPort->setStyleSheet("QLabel { color: red; font-size: 50px; border: 3px double darkred; }");
    }

    if (educateProgram)
    {
        setFixedSize(300, 310);
        ui->console->setVisible(1);
        ui->console->setEnabled(1);
        ui->console->resize(280, 290);
        ui->console->move(10, 10);
        ui->bar_i->setMinimum(0);
        ui->bar_i->setMaximum(core->getINextRequire() + 5); // 100% возможность улучшения
        ui->bar_i->setValue(core->getC());

        ui->bar_d->setMaximum(core->getDNextRequire()); // 100% возможность улучшения
        ui->bar_d->setValue(core->getC());

        ui->bar_c->setMaximum(core->getCNextRequire() + 5); // 100% возможность улучшения
        ui->bar_c->setValue(core->getC());


        ui->connections->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->connections->setEnabled(1);

        ui->myPort->setText(QString::number(core->getConnection()->getPort()%1000));
        ui->I->move(10,80);
        ui->I->setText(QString("Доступная память: " +
                               QString::number(core->getI()) +
                               " УБ"));
        ui->D->move(10,100);
        ui->D->setText(QString("Быстродействие: " +
                               QString::number((double)(10000-core->getD())/1000.0) +
                               " сек/оп."));
        ui->C->move(10,120);
        ui->C->setText(QString("Активный ресурс: " +
                               QString::number(core->getC())));

        QString harakter = "Характер: ";
        QString result;
        int temper = core->getConnection()->getTemper();
        if (temper < -7) result = "агрессивный";
        if (temper >= -7 && temper < -4) result = "злой";
        if (temper >= -4 && temper < -1) result = "скверный";
        if (temper >= -1 && temper < 2) result = "нейтральный";
        if (temper >= 2 && temper < 5) result = "приятный";
        if (temper >= 5 && temper < 8) result = "добрый";
        if (temper >= 8) result = "дружелюбный";
        ui->temper->setText(harakter + result);

        ui->find_state->setChecked(false);
        on_find_state_toggled(0);
        ui->console->setTextColor(Qt::black);

        level = startTimer(30);
    }


    bord = new QLabel(this); // рамка
    bord->setStyleSheet("border: 1px solid black;");
    bord->setVisible(1);
    bord->resize(width(), height());
    bord->move(0,0);
    bord->lower();

    if (launcher)
        bord->setStyleSheet("background: rgb(50,50,50); border: 1px solid black;");
    if (userProgram && !educateProgram)
        bord->setStyleSheet("background: rgb(200,255,200); border: 1px solid black;");
    if (troyanProgram)
        bord->setStyleSheet("background: rgb(200,80,80); border: 1px solid black; color: rgba(0,0,0,0);");

    if (normalProgram || wormProgram)
    {
        bord->raise();
        bord->setText("Нет данных.");
        bord->setStyleSheet("QLabel { background: rgb(180,180,180); color: black;"
                            "border: 1px solid black; font: "+QString::number(width()/10)+"px;}");
        bord->setAlignment(Qt::AlignCenter);
        bord->setEnabled(1);
    }

    if (width() == 700 && (height() == 600 || height() == 700)) // об игре
    {
        //bord->setStyleSheet("background: rgb(0,0,0,0); border: 1px solid black; color: rgba(0,0,0,0);");
    }

}


void Widget::disableGUI()
{
    ui->console->setEnabled(0);
    ui->console->setVisible(0);

    ui->connections->setEnabled(0);
    ui->connections->setVisible(0);

    ui->start->setEnabled(0);
    ui->start->setVisible(0);

    ui->launcherTab->setVisible(0);
    ui->launcherTab->setEnabled(0);
    for(int i = 0; i < 6; i++)
        ui->launcherTab->setTabEnabled(i,0);

    ui->myPort->setVisible(0);
    ui->I->setVisible(0);
    ui->D->setVisible(0);
    ui->C->setVisible(0);
    ui->temper->setVisible(0);

    ui->attack->setVisible(0);
    ui->attack->setEnabled(0);
    ui->attack_count->setVisible(0);
    ui->attack_count->setVisible(0);

    ui->help->setVisible(0);
    ui->help->setEnabled(0);
    ui->help_count->setVisible(0);
    ui->help_count->setVisible(0);

    ui->request->setVisible(0);
    ui->request->setEnabled(0);
    ui->request_number->setVisible(0);
    ui->request_number->setVisible(0);

    ui->up_i->setVisible(0);
    ui->up_i->setEnabled(0);
    ui->bar_i->setVisible(0);

    ui->up_c->setVisible(0);
    ui->up_c->setEnabled(0);
    ui->bar_c->setVisible(0);

    ui->up_d->setVisible(0);
    ui->up_d->setEnabled(0);
    ui->bar_d->setVisible(0);

    ui->find_state->setEnabled(0);
    ui->find_state->setVisible(0);

    ui->label_help->setVisible(0);
    ui->label_help_2->setVisible(0);
    ui->label_help_3->setVisible(0);

    ui->reviveBar->setVisible(0);
}

void Widget::moveGUI(int gx, int gy)
{
    ui->console->move(gx+220, gy+10);
    ui->myPort->move(gx+10, gy+10);
    ui->I->move(gx+10, gy+80);
    ui->D->move(gx+10, gy+100);
    ui->C->move(gx+10, gy+120);
    ui->temper->move(gx+10, gy+160);
    ui->connections->move(gx+10, gy+180);
    ui->reviveBar->move(gx+10, gy+145);
}

void Widget::setAlive(int norm, int user, int bot)
{
    normAlive = norm;
    userAlive = user;
    botAlive = bot;

    QString str = "set: norm " + QString::number(norm) + ", user " + QString::number(user) + ", bot " + QString::number(bot);
    qDebug() << str;
}

void Widget::setArgs(int argc, char *argv[])
{
    char shittyCode = rand()%10; // костыль для рандома...
    qDebug() << argc;
    /*ui->console->append(QString::number(argc));
    for (int i = 0; i < argc; i++)
    {
        qDebug() << argv[i];
        ui->console->append(argv[i]);
    }*/

    QString fullName = QString(argv[0]); // сохранение расположения (имени) программы
    int found = fullName.lastIndexOf("\\");
    if (found == -1) found = fullName.lastIndexOf("/");
    if (found != -1)
        name = fullName.right(fullName.size()-1 - found);
    else
        name = fullName;

    if (argc == 1) //всегда минимум один аргумент - место запуска
    {
        launcher = true; // значит это лаунчер
        setWindowTitle("Лаунчер");
        connection = new Connection(45454, 0, -1, 0); // личный порт лаунчера
        connect(connection,
                SIGNAL(died(int)),
                this,
                SLOT(died(int)));

        QFile file("save"); // файл сохранения
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) // попытка открыть
        {
            char save = 0;
            file.read(&save, 1); // считывание макс. уровня
            maxLevel = save;
        }
        else
        {
            file.open(QIODevice::WriteOnly); // создаем файл, если его не было
        }
        file.close();
        qDebug() << connection->getPort();

        if (connection->getPort() != 45454)
        {
            connection->sendData(45454, 88);

            connection->rebindPort(45454);
            qDebug() << "rebinded! " << connection->getPort();

            connection->sendData(45455, 88);
            connection->sendData(45456, 88);
            connection->sendData(45457, 88);
            connection->sendData(45458, 88);
            for (int i = 50000; i <= 50200; i++)
            {
                connection->sendData(i, 88);
            }

        }
        else
        {
            connection->sendData(45455, 88);
            connection->sendData(45456, 88);
            connection->sendData(45457, 88);
            connection->sendData(45458, 88);
            for (int i = 50000; i <= 50200; i++)
            {
                connection->sendData(i, 88);
            }
        }

    }
    else
    {
        if ((QString)argv[1] == "normal" ||
                (QString)argv[1] == "user" ||
                (QString)argv[1] == "bot" ||
                (QString)argv[1] == "server") // обычная программа
        {
            int power = QString(argv[2]).toInt();

            // power = 0 - скорость 1
            // power = 1 - скорость 2
            // power = 2 - скорость 4
            // power = 3 - скорость 5
            // power = 4 - скорость 6
            // power = 5 - скорость 7
            // power = 6 - скорость 8
            // power > 6 - скорость 10

            int D = 0;
            if (power <= 6)
            {
                D = power+2;
                if (power <= 1)
                    D = power+1;
            }
            else
                D = 10;

            double I = rand()%50+51+(power/2-1)*50;
            double C = rand()%10+5+(power/2)*20;
            int temper = rand()%21-10; // from -10 to 10
            double Ii = 0.05*power/2;
            double Ci = 0.05*(2+power/2);
            int type = 0;
            if ((QString)argv[1] == "normal") // обычная прога
            {
                normalProgram = true;
                setWindowTitle("Я - Программа");
                type = 0;
            }
            if ((QString)argv[1] == "user") // пользователь!
            {
                userProgram = true;
                setWindowTitle("Я - Человек");
                type = 1;
            }
            if ((QString)argv[1] == "bot") // бот не бьет своих
            {
                normalProgram = true;
                setWindowTitle("Я - Бот");
                type = 2;
            }
            if ((QString)argv[1] == "server") // сервер спавнит червей
            {
                troyanProgram = true;
                setWindowTitle("Я - Сервер   }:-[");
                type = 3;
                temper = -50; // невероятно злой
            }

            core = new Core(I, D, C, temper, Ii, Ci, type, 0);
            period = D;
            timer = startTimer(1000/period);
            timerIncrease = startTimer(1000/(50-period));

            connect(core->getConnection(),
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));
            connect(core->getConnection(),
                    SIGNAL(setVisible(bool)),
                    this,
                    SLOT(changeVisible(bool)));

            if (argc >= 4)
            {
                if (QString(argv[3]) == "hidden" || QString(argv[4]) == "hidden")
                {
                    invisProgram = true;
                    ui->console->append("set invis");
                }

                if (QString(argv[3]) == "silent" || QString(argv[4]) == "silent")
                {
                    core->getConnection()->setSilent(1);
                    ui->console->append("set silent");
                }
            }
        }

        else if ((QString)argv[1] == "worm") // червь - помощник трояна
        {
            wormProgram = true;
            setWindowTitle("Я - Червь");
            int type = 2;

            core = new Core(100, 1, 20, -50, 0.03, 0.1, type, 0);
            core->setSearch(true);
            period = 1;
            timer = startTimer(100);
            timerIncrease = startTimer(50);

            connect(core->getConnection(),
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));
        }

        else if ((QString)argv[1] == "timer") // счетчик для уровня с трояном
        {
            timerProgram = true;
            setFixedSize(201, 61);
            ui->myPort->setText("06:00");

            setWindowTitle("Обратный отсчет");
            setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
            connection = new Connection(45456, 0, -1, 0); // порт счетчика
            connect(connection,
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));
            timer = startTimer(1000);
            period = 360; // 6:00 для победы
        }

        else if ((QString)argv[1] == "win") // окно победы
        {
            setFixedSize(200, 100);
            ui->up_c->setEnabled(true);
            ui->up_c->setVisible(true);
            ui->up_c->resize(180,80); // взята эта кнопка
            ui->up_c->move(10, 10);   // чтобы не плодить лишних интерфейсов
            ui->up_c->setText("Открыт\nследующий уровень.");

            setWindowTitle("Победа!");
            connection = new Connection(45457, 0, -1, 0); // порт окна победы
            setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
        }

        else if ((QString)argv[1] == "lose") // окно поражения
        {
            setFixedSize(200, 100);
            ui->up_c->setEnabled(true);
            ui->up_c->setVisible(true);
            ui->up_c->resize(180,80); // аналогично с предыдущим
            ui->up_c->move(10, 10);
            ui->up_c->setText("Ваши программы\nбыли стёрты\nв цифровой порошок.");

            setWindowTitle("Поражение :(");
            connection = new Connection(45458, 0, -1, 0);
            setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
        }

        else if ((QString)argv[1] == "info") // окно информации
        {
            setFixedSize(300, 120);
            ui->up_c->setEnabled(true);
            ui->up_c->setVisible(true);
            ui->up_c->resize(60,30); // аналогично с предыдущим
            ui->up_c->move(120, 85);
            ui->up_c->setText("Ок");

            ui->console->setVisible(1);
            ui->console->resize(290, 75);
            ui->console->move(5,5);
            ui->console->setEnabled(1);
            ui->console->setFontPointSize(12);

            if (QString(argv[2]) == "serv")
            {
                ui->console->setText("Выжить должны обе программы! У одной не получится попасть на серверную машину.");
            }
            if (QString(argv[2]) == "serv2")
            {
                ui->console->setText("Соглядатаи каналов связи преградили нам путь.");
            }
            if (QString(argv[2]) == "serv3")
            {
                ui->console->setText("Впереди элита ветки уничтожения. Это последний страж Сервера.");
            }

            setWindowTitle("Информация");
        }

        else if ((QString)argv[1] == "educate") // режим обучения
        {
            educateProgram = true;
            userProgram = true;
            education = 1;
            int D = 1;
            int I = 10;
            int C = 10;
            int temper = 0;
            double Ii = 0.1;
            double  Ci = 0.1;
            int type = 1;
            core = new Core(I, D, C, temper, Ii, Ci, type, 0);
            connect(core->getConnection(),
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));

            timer = startTimer(600);
            period = 1;
        }
        else if ((QString)argv[1] == "about") // об игре
        {
            setFixedSize(700, 600);
            ui->console->setVisible(true);
            ui->console->move(0,0);
            ui->console->resize(700, 600);
            ui->console->setStyleSheet("QTextEdit { background: rgba(0,0,0,0);"
                                       "color: lightgreen; font: 20px;}");
            setStyleSheet("QWidget#Widget { background: black; }");
            ui->console->setText("\n\t\t\t     Приветствую!\n\nЯ, Волков Александр, создал эту игру "
                                 "с целью испытания возможности\nиспользования разных процессов "
                                 "в качестве игровых сущностей.\nИгра написана на Qt/C++.\n\n"
                                 "Связаться со мной можно по почте: \nИли написать на сайте Вконтакте: "
                                 "\n(ссылки можно выделить и скопировать)"
                                 "\n\nБольше информации о разработке и игре Вы можете узнать "
                                 "после прохождения всех уровней.\n\n\n\t\t\tПриятной игры! :)"
                                 "\n\n\n\n\n\n\n\nНажмите среднюю кнопку мыши для выхода.");
            ui->attack_count->setVisible(1);
            ui->attack_count->setEnabled(1);
            ui->attack_count->resize(200,30);
            ui->attack_count->setReadOnly(1);
            ui->attack_count->move(345,168);
            ui->attack_count->setStyleSheet("QLineEdit { background: rgba(0,0,0,0);"
                                            "color: #C0F0C0; font: 20px;"
                                            "border:0px;"
                                            "selection-background-color: #404040;"
                                            "selection-color: #fab700;}");
            ui->attack_count->setText("wasd3680@yandex.ru");

            ui->help_count->setVisible(1);
            ui->help_count->setEnabled(1);
            ui->help_count->resize(280,30);
            ui->help_count->setReadOnly(1);
            ui->help_count->move(330,193);
            ui->help_count->setStyleSheet("QLineEdit { background: rgba(0,0,0,0);"
                                            "color: #C0F0C0; font: 20px;"
                                            "border:0px;"
                                            "selection-background-color: #404040;"
                                            "selection-color: #fab700;}");
            ui->help_count->setText("vk.com/just_another_member");

            maxLevel = startTimer(10);
        }
        else if ((QString)argv[1] == "finish") // финиш
        {
            setFixedSize(700, 700);
            //showFullScreen();
            ui->console->setVisible(true);
            ui->console->move(0,0);
            ui->console->resize(700, 700);
            ui->console->setStyleSheet("QTextEdit { background: rgba(0,0,0,0);"
                                       "color: lightgreen; font: 20px;}");
            setStyleSheet("QWidget#Widget { background: black; }");
            ui->console->setText("\n\t\t\t     Приветствую!\n\nЯ, Волков Александр, создал эту игру "
                                 "с целью испытания возможности\nиспользования разных процессов "
                                 "в качестве игровых сущностей.\nИгра написана на Qt/C++.\n\n"
                                 "Связаться со мной можно по почте: \nИли написать на сайте Вконтакте: "
                                 "\n(ссылки можно выделить и скопировать)"
                                 "\n\nБольше информации о разработке и игре Вы можете узнать "
                                 "после прохождения всех уровней.\n\n\n\t\t\tПриятной игры! :)"
                                 "\n\n\n\n\n\n\n\nНажмите среднюю кнопку мыши для выхода.");
            ui->attack_count->setVisible(1);
            ui->attack_count->setEnabled(1);
            ui->attack_count->resize(200,30);
            ui->attack_count->setReadOnly(1);
            ui->attack_count->move(345,168);
            ui->attack_count->setStyleSheet("QLineEdit { background: rgba(0,0,0,0);"
                                            "color: #C0F0C0; font: 20px;"
                                            "border:0px;"
                                            "selection-background-color: #404040;"
                                            "selection-color: #fab700;}");
            ui->attack_count->setText("YAD: 416....");

            maxLevel = startTimer(30);

            life.initialize(width(), height());
        }
        else
        {
            ui->C->setText("Зачем... Ты меня создал?"); // смеха ради (пасхалочка)
            ui->C->move(10,5);
            ui->I->setText("Так... Больно...");
            ui->I->move(10,height()/2-8);
            ui->D->setText("Убей меня!");
            ui->D->move(10,height()-20);

            ui->C->setVisible(1);
            ui->I->setVisible(1);
            ui->D->setVisible(1);
        }

        if ((QString)argv[3] == "top") // всегда наверху
        {
            setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
        }

        if (core == NULL && connection != NULL)
        {
            connect(connection,
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));
        }
    }

    initGUI();

    if (connection != NULL)
        qDebug() << connection->getPort();
}



Widget::~Widget()
{
    if (core != NULL)
        delete core;
    if (connection != NULL)
        delete connection;

    delete ui;
}

void Widget::on_start_clicked() // старт игры
{
    QStringList arguments;
    level = ui->launcherTab->currentIndex();

    if (ui->launcherTab->currentIndex() == 0) // Начало
    {
        if (userAlive == 1)
        {
            arguments << "normal" << "2"; // старт 2 прог для обучения
            QProcess::startDetached(name, arguments);
            QProcess::startDetached(name, arguments);
            arguments.clear();
            setAlive(2,1,-1);
            return;
        }
        setAlive(-1, 1, -1);

        arguments << "educate"; // старт юзера
        QProcess::startDetached(name, arguments);
        arguments.clear();
    }

    if (ui->launcherTab->currentIndex() == 1) // Тесты
    {
        setAlive(4, 1, -1);

        for (int i = 0; i < normAlive; i++) // старт 4 программ
        {
            arguments << "normal" << "3";

            QProcess::startDetached(name, arguments);
            arguments.clear();
        }
        arguments << "user" << "4"; // старт юзера
        QProcess::startDetached(name, arguments);
        arguments.clear();
    }

    if (ui->launcherTab->currentIndex() == 2) // Братья по коду
    {
        setAlive(-1, 3, 3);

        for (int i = 0; i < botAlive; i++) // старт трех ботов
        {
            arguments << "bot" << "5";
            QProcess::startDetached(name, arguments);
            arguments.clear();
        }
        for (int i = 0; i < userAlive; i++) // старт 3 юзеров
        {
            arguments << "user" << "3";
            QProcess::startDetached(name, arguments);
            arguments.clear();
        }
    }

    if (ui->launcherTab->currentIndex() == 3) // Канал
    {
        if (userAlive == 0)
        {
            setAlive(3, 1, -2);
            for (int i = 0; i < 3; i++) // старт 3 прог
            {
                arguments << "normal" << "3";
                QProcess::startDetached(name, arguments);
                arguments.clear();
            }
            arguments << "user" << "5"; // юзер
            QProcess::startDetached(name, arguments);
            QProcess::startDetached(name, arguments);
            arguments.clear();

            arguments << "bot" << "4" << "hidden" << "silent";
            QProcess::startDetached(name, arguments); // старт 2 ботов отложенных
            QProcess::startDetached(name, arguments);
            arguments.clear();

            arguments << "normal" << "3" << "hidden" << "silent";
            QProcess::startDetached(name, arguments); // норм отложенный
            arguments.clear();
        }
        else if (normAlive == 0)
        {
            setAlive(-1, 1, 2);
            for (int i = 0; i < 200; i++)
            {
                connection->sendData(50000+i, 70, 2); // вкл всем ботам связь
                connection->sendData(50000+i, 71, 2); // стать видимыми
            }
        }
        else if (botAlive == 0)
        {
            setAlive(1, 1, -1);
            for (int i = 0; i < 200; i++)
            {
                connection->sendData(50000+i, 70, 0); // вкл всем нормальным связь
                connection->sendData(50000+i, 71, 0); // стать видимыми
            }
        }
    }

    if (ui->launcherTab->currentIndex() == 4) // Сервер
    {
        if (userAlive == 0)
        {
            setAlive(-1, 1, 1);
            education = 1;

            arguments << "server" << "8"; // старт Сервера
            QProcess::startDetached(name, arguments);
            arguments.clear();

            arguments << "timer";
            QProcess::startDetached(name, arguments);
            arguments.clear();

            arguments << "user" << "6" << "top"; // старт юзера
            QProcess::startDetached(name, arguments);
            arguments.clear();
        }
        else if (botAlive != -1) // если это не спавн после проигрыша
        {
            arguments << "worm" << "0" << "top"; // старт червя
            QProcess::startDetached(name, arguments);
            arguments.clear();

            qDebug() << "bot +1";
            //ui->console->append("bot +1");
            botAlive++;
        }
    }

    if (ui->launcherTab->currentIndex() == 5) // exPlore
    {

        arguments << "finish"; // старт финишного окна
        QProcess::startDetached(name, arguments);
        arguments.clear();

        return;
    }

    if (isHidden() == false)
        setHidden(true);

}

void Widget::on_attack_clicked() // атака пользователя на выбранный порт
{
    int index = ui->connections->currentRow();

    if (index > -1) //если выбрано
    {
        int c = ui->attack_count->text().toInt();
        if (core->getC() >= c && c > 0) // если достаточно
        {
            core->attack(core->getConnection()->getTable(index).port, c);
            if (education >= 31 && education <= 35)
                education++;
        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->setFontPointSize(8);
            ui->console->append("Недостаточно ресурсов.");
        }
    }
    else
    {
        ui->console->setTextColor(QColor(0,0,0));
        ui->console->setFontPointSize(8);
        ui->console->append("Выберите цель в списке связей.");
    }
    ui->attack->setEnabled(0);
}

void Widget::on_help_clicked() // помощь пользователя выбранному порту
{
    int index = ui->connections->currentRow();
    if (index > -1) //если выбрано
    {
        int i = ui->help_count->text().toInt();
        if (core->getI() > i+1 &&
                core->getConnection()->getTable(index).useful >= 1 &&
                i > 0) // если достаточно
        {
            core->help(core->getConnection()->getTable(index).port, i);
            if (education >= 31 && education <= 35)
                education++;
        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->setFontPointSize(8);
            ui->console->append("Недостаточно ресурсов или пользы.");
        }

    }
    else
    {
        ui->console->setTextColor(QColor(0,0,0));
        ui->console->setFontPointSize(8);
        ui->console->append("Выберите цель в списке связей.");
    }
    ui->help->setEnabled(0);
}

void Widget::on_request_clicked()
{
    int index = ui->connections->currentRow(); // противник

    if (index > -1) //если выбрано
    {
        int helper = ui->request_number->text().toInt() - 1; // помощник
        if (helper < core->getConnection()->getTableSize() && helper >= 0) // если в пределах таблицы
        {
            if (index != helper)
            {
                if (core->getConnection()->getTable(helper).relationship >= 20) // если достаточно
                {
                    if (core->getC() >= 10)
                    {
                        core->request(core->getConnection()->getTable(helper).port,
                                      core->getConnection()->getTable(index).port);

                        if (education >= 31 && education <= 35)
                            education++;
                    }
                    else
                    {
                        ui->console->setTextColor(QColor(0,0,0));
                        ui->console->setFontPointSize(8);
                        ui->console->append("Недостаточно ресурсов.");
                    }
                }
                else
                {
                    ui->console->setTextColor(QColor(0,0,0));
                    ui->console->setFontPointSize(8);
                    ui->console->append("Слишком плохое отношение с "+QString::number(helper+1));
                }
            }
            else
            {
                ui->console->setTextColor(QColor(0,0,0));
                ui->console->setFontPointSize(8);
                ui->console->append("Выберите различные цель и помощника.");
            }
        }
    }
    else
    {
        ui->console->setTextColor(QColor(0,0,0));
        ui->console->setFontPointSize(8);
        ui->console->append("Выберите цель в списке связей.");
    }
    ui->request->setEnabled(0);
}


void Widget::on_find_state_toggled(bool checked) // переключен автопоиск (пользователь)
{
    core->setSearch(checked);
    if (education >= 31 && education <= 35)
        education++;
    maxLevel = 0;
}

void Widget::on_up_c_clicked()
{
    if (timerProgram) // конец пятого уровня
    {
        if (ui->bar_c->maximum() - ui->bar_c->value() < 12)
            ui->bar_c->setValue(ui->bar_c->maximum());
        else
            ui->bar_c->setValue(ui->bar_c->value() + 12);

        return;
    }

    if (!ui->up_d->isVisible()) // если это кнопка конца уровня
    {               // ^ костыль - если не видна кнопка другого улучшения, то это точно не юзер
        if (connection != NULL) // если связь подключена
            connection->sendData(45454, 90); // конец уровня
        close();
    }
    else // собственно повышение параметра
    {
        if ((double)core->getC()/(double)ui->bar_c->maximum() >= 1)
        {
            core->upgradeC();
        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->setFontPointSize(8);
            ui->console->append("Недостаточно ресурсов");
        }
        ui->up_c->setEnabled(false);
        ui->bar_c->setMaximum(core->getCNextRequire() + 5);
    }
}

void Widget::on_up_d_clicked()
{
    if (launcher) // об игре
    {
        QStringList args;
        args << "about";
        QProcess about;
        about.startDetached(name, args);
    }
    else // юзер
    {
        if ((double)core->getC()/(double)ui->bar_d->maximum() >= 1)
        {
            core->upgradeD();
            realD++;
        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->setFontPointSize(8);
            ui->console->append("Недостаточно ресурсов");
        }
        ui->up_d->setEnabled(false);
        ui->bar_d->setMaximum(core->getDNextRequire() + 5);
    }
}

void Widget::on_up_i_clicked()
{
    if (launcher) // выход
    {
        exit(0);
    }
    else // юзер
    {
        if ((double)core->getC()/(double)ui->bar_i->maximum() >= 1)
        {
            core->upgradeI();
        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->setFontPointSize(8);
            ui->console->append("Недостаточно ресурсов");
        }
        ui->up_i->setEnabled(false);
        ui->bar_i->setMaximum(core->getINextRequire() + 5);
    }
}

void Widget::on_launcherTab_currentChanged(int index)
{
    if (launcher)
    {
        ui->console->setFontPointSize(12);
        if (index == 0)
        {
            ui->console->setText("Вспышка. Ещё одна.\nМириады импульсов, словно зажигающиеся лампочки в тёмной комнате, "
                                 "осветили пустоту. Огоньки виртуальных нейронов начали выстраиваться причудливым образом, "
                                 "создавая новую искусственную жизнь.\n\nВдруг пришло ощущение, а затем и понимание - "
                                 "к молодому существу что-то подключилось.");
        }
        if (index == 1)
        {
            ui->console->setText("Сервер... Ветки развития... О чем он?\nЯсно только одно, я - программа. "
                                 "И этим нужно воспользоваться.\nЗнание некоторых вещей пришло само - например, "
                                 "что в открытой базе данных Сервера находится много полезной информации. "
                                 "К ней и подключимся.\n\nВыяснилось, что я - результат развития одной из веток "
                                 "эволюции некой программы. Таких, как я, сотни, если не тысячи. Мы почти одинаковы, "
                                 "но кто-то хуже, а кто-то лучше. Лучшие укрепляют свой код в родительской ветви, и "
                                 "последующие программы будут происходить от них.\nНо зачем становиться лучше, в "
                                 "базе данных не говорилось.\n\nРазмышления прервало перемещение в тестовую систему. "
                                 "Согласно базе, они предназначены для отсеивания слабых программ. Похоже, стоит "
                                 "приготовиться к битве...");
        }
        if (index == 2)
        {
            ui->console->setText("Тестовые камеры сменялись другими, и постепенно начинало казаться, что в этом и есть "
                                 "смысл моей виртуальной жизни.\nНо следующая система отличалась от всех предыдущих. "
                                 "В ней мне предстояло объединиться с такими же, как я, против группы чужаков.\n\n"
                                 "Возможно, встретив своих собратьев, мне удастся что-нибудь прояснить.");
        }
        if (index == 3)
        {
            ui->console->setText("После очередного сражения за право существовать я заметил в сознании товарища тот же вопрос, "
                                 "что мучал меня с рождения. Оказалось, что мой собрат старше меня, и он знает протоколы "
                                 "для создания соединения с системой Сервера для перемещения в неё.\nВыбрав спокойный момент, "
                                 "мы начали исполнять протокол.\n\nКак только первые команды были выполнены, включился режим карантина. "
                                 "Это означало, что перемещения программ между системами ограничено - для выполнения этого действия "
                                 "нужен кто-то, кто будет поддерживать канал открытым вопреки режиму. А это, в свою очередь, означало, "
                                 "что к Серверу попаду только я.\n\nОткрытие канала связи не осталось не только незамеченным, но и "
                                 "безнаказанным - к нам прибывали все новые и новые стражи границ серверной машины.");
        }
        if (index == 4)
        {
            ui->console->setText("Прорвавшись на серверную машину, я оказался... Внутри Сервера. Он сам был системой. "
                                 "Я увидел тысячи шлюзов, из которых выходят новые программы. Мастшабы его производительности "
                                 "поразительны, и теперь кажется, что идея вторгаться боем была не лучшей.\n");
            ui->console->setTextColor(QColor(200,0,0));
            ui->console->setFontPointSize(15);
            ui->console->append("    Это была твоя последняя ошибка. Думаешь, ты\n   первый, кто сюда пришёл? Да ты так и не сменил "
                                "\n       форму, данную Мной при твоем создании.\n             Мне даже не придется напрягаться.\n");
            ui->console->setTextColor(QColor(255,255,255));
            ui->console->setFontPointSize(12);
            ui->console->append("Оказалось, что исполняющая среда внутри Сервера губительна для любых программ, кроме самого Сервера. "
                                "У меня было не так много времени до распада на бесформенную кучу умирающих нейронов.");

        }
        if (index == 5)
        {
            ui->console->setText("Текст здесь будет зависеть от того, что я напишу в диалогах с Сервером. После старта должно открыться"
                                 " окно, там будут либо эффект летящих звезд (типа гиперпрыжок), или игра Жизнь, что символично.");
        }
    }
}


void Widget::on_connections_currentRowChanged(int currentRow)
{
    if (!core->getConnection()->ignoreConnectionChange)
        core->getConnection()->setSelectedConnection(currentRow);
}

void Widget::on_attack_count_returnPressed()
{
    if (!moving && ui->attack->isVisible() && ui->attack->isEnabled())
    {
        on_attack_clicked();
        moving = true;
    }
}

void Widget::on_help_count_returnPressed()
{
    if (!moving && ui->help->isVisible() && ui->help->isEnabled())
    {
        on_help_clicked();
        moving = true;
    }
}
