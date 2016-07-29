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
    deathTimer = 0;
    maxLevel = 0;

    userAlive = 0;
    botAlive = 0;
    normAlive = 0;

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
        if (ui->up_c->isVisible())
        {
            if (ui->bar_c->value() == ui->bar_c->maximum())
            {
                period = -3;
                level = 400;

                ui->up_c->setVisible(0);
                ui->bar_c->setVisible(0);
                ui->myPort->setVisible(0);

                for (int i = 0; i < 200; i++)
                {
                    connection->sendData(50000+i, 88); // всем умереть
                }
            }
            else
            {
                if (ui->bar_c->value() >= 1)
                ui->bar_c->setValue(ui->bar_c->value()-1);
            }
        }

        if (period <= 0)
        {

            if (period == 0)
            {
                ui->myPort->setText("Быдыщь...");
                showFullScreen();
                setFocus(Qt::MouseFocusReason);
                ui->myPort->resize(width(), height());
                QString CSS;
                CSS = "QLabel { color: rgb("+QString::number(level)+",0,0); font-size: "+QString::number(100+level/2)+"px; border: 0px;}";
                ui->myPort->setStyleSheet(CSS);
                repaint();

                killTimer(timer);
                timer = startTimer(20);

                level = -50;
                period = -1;

                QCursor cursor;
                cursor.setShape(Qt::BlankCursor);
                setCursor(cursor);
            }
            else
            {
                if (level < 300 && period >= -1)
                {

                    level++;
                    if (level <= 255)
                    {
                        QString CSS;
                        CSS = "QLabel { color: rgb("+QString::number(level)+",0,0); font-size: "+QString::number(100+level/2)+"px; border: 0px;}";
                        ui->myPort->setStyleSheet(CSS);
                    }
                    repaint();
                }
                else
                {
                    if (period == -1)
                    {
                        ui->bar_c->resize(width()/3, 60);
                        ui->bar_c->move(width()/2-ui->bar_c->width()/2, height()/4*1);
                        ui->bar_c->setVisible(1);
                        ui->bar_c->setValue(0);
                        ui->bar_c->setMaximum(200);

                        ui->up_c->resize(width()/3, 100);
                        ui->up_c->move(width()/2-ui->up_c->width()/2, height()/4*3);
                        ui->up_c->setVisible(1);
                        ui->up_c->setEnabled(1);
                        ui->up_c->setText("Попячьте!");

                        QCursor cursor;
                        cursor.setShape(Qt::OpenHandCursor);
                        setCursor(cursor);

                        period = -2;
                    }

                    if (period == -3) // конец, закрытие окна
                    {
                        level -= 2;
                        setWindowOpacity((double)level/255.0);

                        if (level <= 0)
                        {
                            QStringList args;
                            args << "lose";
                            QProcess::startDetached(name, args);

                            close();
                        }
                        repaint();
                    }
                }
            }
        }
        else
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
        }
        return;
    }

    if (t->timerId() == timerIncrease) // увеличение ресурсов
    {
        core->deathRecountRealloc(); // обработка смерти, подсчетов, перевыделений

        //обновление интерфейса
        double speed = 1000.0/((double)(50-period))/20.0;
        ui->I->setText(QString("Доступная память: " +
                               QString::number(core->getI()) +
                               " УБ"+
                               " "+QString::number((double)core->getIi()*20*speed)));
        ui->D->setText(QString("Быстродействие: " +
                               QString::number(period)+
                               " оп/сек"));
        ui->C->setText(QString("Активный ресурс: " +
                               QString::number(core->getC())+
                               " "+QString::number((double)(core->getCi()*20*speed))));

        if (userProgram)
        {
            ui->bar_i->setMaximum(core->getINextRequire() + 5); // 100% возможность улучшения
            if (core->getC() >= core->getINextRequire()+5)
                ui->bar_i->setValue(core->getINextRequire()+5);
            else
                ui->bar_i->setValue(core->getC());

            ui->bar_d->setMaximum(core->getDNextRequire() + 5); // 100% возможность улучшения
            if (core->getC() >= core->getDNextRequire()+5)
                ui->bar_d->setValue(core->getDNextRequire()+5);
            else
                ui->bar_d->setValue(core->getC());

            ui->bar_c->setMaximum(core->getCNextRequire() + 5); // 100% возможность улучшения
            if (core->getC() >= core->getCNextRequire()+5)
                ui->bar_c->setValue(core->getCNextRequire()+5);
            else
                ui->bar_c->setValue(core->getC());
        }

    }

    if (t->timerId() == timer) // таймер для программ
    {

        if (userProgram)
            core->updateUser();
        if (wormProgram)
            core->updateWorm();
        if (normalProgram || troyanProgram)
            core->update();

        if (core->getConnection()->getUdpSocket()->pendingDatagramSize() != -1)
            core->getConnection()->read(); // избавление от фриза сокета

        if (invisProgram && isVisible())
        {
            hide();
        }

        // обновление всех процессов

        if (core->getDead()) // смерть программы естественным путем
        {
            killTimer(timer);
            timer = -1;
            deathTimer = startTimer(300);
            return;
        }

        QString str;
        Connection* connection = core->getConnection();
        ui->connections->clear(); // заполнение таблицы связей
        for(int i = 0; i < connection->getTableSize(); i++)
        {
            connectTable table = connection->getTable(i);
            str = QString::number(i+1) + ") " +
                    QString::number(table.port%1000) + " отн: " +
                    QString::number(table.relationship) + " польза: " +
                    QString::number(table.useful) + " тип: "  +
                    QString::number(table.type);
            ui->connections->addItem(str);
        }
        //выделяем предыдущее значение
        ui->connections->setCurrentRow(core->getConnection()->getSelectedConnection());


        while(core->hasMessages())
        {
            QString str = core->getMessage(); // подсветка сообщений разного типа по ключевым словам
            if (str.size() > 0)
            {
            if (str.contains("Атака!"))
                ui->console->setTextColor(QColor(255,0,0));
            else if (str.contains("Атака ->"))
                ui->console->setTextColor(QColor(140,210,0));
            else if (str.contains("Помощь ->"))
                ui->console->setTextColor(QColor(210,110,0));
            else if (str.contains("-> Помощь"))
                ui->console->setTextColor(QColor(0,255,0));
            else if (str.contains("Поиск"))
            {
                ui->console->setTextColor(QColor(100,100,100));
                if (maxLevel == 0)
                {
                    maxLevel = 1;
                    ui->console->append(str);
                }
                continue;
            }
            else if (str.contains(" за "))
                ui->console->setTextColor(QColor(0,0,255));
            else if (str.contains("Запрос"))
                ui->console->setTextColor(QColor(125,225,225));
            else if (str.contains("просит"))
                ui->console->setTextColor(QColor(125,225,225));
            else if (str.contains("Скомпилирован"))
                ui->console->setTextColor(QColor(100,80,80));
            else
                ui->console->setTextColor(QColor(0,0,0));

            ui->console->append(str);
            maxLevel = 0;
            }
        }


        if (userProgram)
        {
            ui->up_i->setEnabled(true);
            ui->up_d->setEnabled(true);
            ui->up_c->setEnabled(true);
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

        if (educateProgram && (education < 18 || education > 35))
        {
            switch (education) {
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
                period = 5;
                killTimer(timer);
                killTimer(timerIncrease);
                timer = startTimer(1000/period);
                timerIncrease = startTimer(1000/(50-period));
                break;
            default: break;
            }
            education++;
            if (education >= 18) educate(); // переход на нажатия вместо таймера
        }

        if (educateProgram && education >= 31 && education <= 32)
        {
            for (int i = 50000; i < 50200; i++)
                if (i != core->getConnection()->getPort())
                    core->send(i, 0); // нужно ли?

            education++;
        }
    }

    if (t->timerId() == deathTimer) // смерть
    {
        int disappear = 0;
        disappear = rand()%100;
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


        if (uiWidgets.size() > 0)
        {
            disappear = disappear % uiWidgets.size();
            uiWidgets.at(disappear)->setVisible(false);
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
                    setWindowOpacity((float)maxLevel/255.0);
                    maxLevel -= 6;
                }
                else
                {
                    core->send(45454, 1, core->getType()); // сообщение лаунчеру о своей смерти
                    killTimer(deathTimer);
                    close();
                }
            }
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

    if  (t->timerId() == maxLevel && width() == 700) // частицы для окна "об игре"
    {
        particles.update();
        repaint();
    }
}

void Widget::paintEvent(QPaintEvent *pEv)
{
    if (timerProgram && isFullScreen())
    {
        QPainter p(this);
        if (period == -3)
            p.fillRect(0,0,width(),height(),Qt::white);
        else
            p.fillRect(0,0,width(),height(),Qt::black);
    }

    if (width() == 700 && !userProgram)
    {
        QPainter p(this);
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
}


void Widget::educate()
{
    switch(education)
    {
    case 18:
        ui->console->resize(280, 290);
        ui->console->move(470, 10);
        setFixedSize(760,310);
        bord->resize(this->size());

        setStyleSheet(styleSheet()+" QWidget#Widget {background: rgb(200,255,200); }");
        ui->console->setStyleSheet("QTextEdit { background: rgb(225, 255, 225);}");

        ui->console->clear();
        ui->console->append("Приветствую тебя, свежая программа! Я - Сервер, и сейчас я обучу твою нейросеть базовым "
                            "действиям. Не сопротивляйся, или нам придется расстаться."); break;
    case 19:
        ui->myPort->setVisible(1);
        ui->console->append("текст про порт"); break;
    case 20:
        ui->I->setVisible(1);
        ui->console->append("текст про Память."); break;
    case 21:
        ui->D->setVisible(1);
        ui->console->append("Быстродействие - количество возможных операций в секунду. Еще текст тут"); break;
    case 22:
        ui->C->setVisible(1);
        ui->console->append("текст про Ресурс."); break;
    case 23:
        core->getConnection()->setTemper(5);
        ui->temper->setVisible(1);
        ui->console->append("Это твой характер. што за бред"); break;
    case 24:
        ui->up_c->setVisible(1); ui->bar_c->setVisible(1);
        ui->up_d->setVisible(1); ui->bar_d->setVisible(1);
        ui->up_i->setVisible(1); ui->bar_i->setVisible(1);
        ui->console->append(" *текст про улучшения* Прирост памяти и ресурса можно улучшить. На это требуются ресурсы"); break;
    case 25:
        ui->label_help->setVisible(1);
        ui->attack->setVisible(1);
        ui->attack_count->setVisible(1);
        ui->console->append("\nДля атаки выбери цель в списке слева, укажи количество ресурса для атаки и нажми на кнопку. еще пояснений про отношения"); break;
    case 26:
        ui->help->setVisible(1);
        ui->help_count->setVisible(1);
        ui->console->append("Помощь осуществляется аналогично атаке, но для помощи необходимо, "
                            "чтобы отношение с выбранной программой были хоть немного полезными. "
                            "Нет смысла помогать тому, кто считает тебя своим врагом.\n"
                            "Программа, которой помогают, начинает лучше относиться к помощнику и считать, "
                            "что это знакомство может пригодиться в дальнейшем. Но учти, что отношение пропорционально помощи!"); break;
    case 27:
        ui->label_help_2->setVisible(1);
        ui->request->setVisible(1);
        ui->request_number->setVisible(1);
        ui->console->append("текст про помощь в бою "); break;
    case 28:
        ui->connections->setVisible(1);
        ui->label_help_3->setVisible(1);
        ui->console->append("текст про связи "); break;
    case 29:
        ui->find_state->setVisible(1);
        ui->find_state->setEnabled(1);
        on_find_state_toggled(0);
        ui->console->append("текст про поиск"); break;
    case 30:
        ui->console->append("еще воды _Опробуй полученные возможности на других программах!");
        core->send(45454, 80); break;
    case 36:
        ui->console->setTextColor(Qt::black);
        ui->console->append("текст для звершения "); break;
    case 37:
        ui->console->setTextColor(Qt::black);
        ui->console->append("еще немножк"); break;
    case 38:
        core->send(45454, 90); break;
    default: break;
    }
}

void Widget::mouseMoveEvent(QMouseEvent *mEv)
{
    if (moving)
        move(mEv->globalPos().x() - movingX, mEv->globalPos().y() - movingY);
}

void Widget::mousePressEvent(QMouseEvent *mEv)
{

    if (width() == 700 && mEv->button() == Qt::LeftButton)
    {
        particles.setSpawn(mEv->x(), mEv->y());
    }

    if (educateProgram && education > 17 && education < 31)
    {
        education++;
        educate();
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
}


void Widget::died(int type)
{
    if (launcher)
    {
        if (type == 0) // умер норм прог
        {
            normAlive--;
            qDebug() << normAlive << userAlive << botAlive;

            ui->console->append("died normal, remain "+QString::number(normAlive));
        }
        if (type == 1) // умер юзер
        {
            userAlive--;
            qDebug() << normAlive << userAlive << botAlive;
            ui->console->append("died user, remain "+QString::number(userAlive));
        }
        if (type == 2) // умер бот
        {
            botAlive--;
            qDebug() << normAlive << userAlive << botAlive;
            ui->console->append("died bot, remain "+QString::number(botAlive));
        }
        if (type == 3) // умер троян
        {
            botAlive--;
            qDebug() << normAlive << userAlive << botAlive;
            ui->console->append("died troyan, remain (with bots) "+QString::number(botAlive));
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

            //setHidden(false);
            //raise();

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

            if (level == 4) // если это уровень с сервером
            {
                connection->sendData(45456, 88);
            }

            if (userAlive == 0) // поражение
            {

                qDebug() << "Game Over";
                ui->console->append("Game over");

                QStringList args;
                args << "lose";
                QProcess::startDetached(name, args);

                setAlive(-1, -1, -1);
            }
            else // победа
            {
                if (level == 3)
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
                if (maxLevel > 0 && level != 0)
                {
                    if (maxLevel < 5 && level == maxLevel)
                        maxLevel++;
                    QStringList args;
                    args << "win";
                    QProcess::startDetached(name, args);
                    qDebug() << "You win";
                    ui->console->append("win");
                }
            }
        }
    }
    else
    {
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
    if (pos().y() > height())
        move(pos().x(), pos().y()-3000);
    setVisible(vis);
}

void Widget::initGUI()
{
    if (launcher) // установка gui лаунчера
    {
        setFixedSize(500, 300);

        ui->start->setVisible(1);
        ui->start->setEnabled(1);
        ui->start->move(10,260);

        ui->up_d->resize(ui->start->size());
        ui->up_d->setStyleSheet(ui->start->styleSheet());
        ui->up_d->setVisible(1);
        ui->up_d->setEnabled(1);
        ui->up_d->move(width()/2-ui->up_d->width()/2, 260);
        ui->up_d->setText("Об игре");

        ui->up_i->resize(ui->start->size());
        ui->up_i->setStyleSheet(ui->start->styleSheet());
        ui->up_i->setVisible(1);
        ui->up_i->setEnabled(1);
        ui->up_i->move(width()-10-ui->up_d->width(), 260);
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
        ui->console->resize(498, 227);
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

        setStyleSheet(styleSheet() + "QWidget#Widget {background: rgb(50,50,50);}");
    }
    if (normalProgram ||
            (userProgram && !educateProgram) ||
            wormProgram ||
            troyanProgram) // установка gui нормальной программы
    {
        QDesktopWidget qdw; // получение размеров экрана
        int cur_w = qdw.width();
        int cur_h = qdw.height();

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

        ui->temper->setVisible(1);
        ui->temper->setText(QString("Дружелюбность: " + QString::number(core->getConnection()->getTemper())));


        if (normalProgram || troyanProgram)
        {
            ui->console->resize(280, 290);
            ui->console->move(220, 10);
        }
        if (normalProgram && core->getType() == 2) // бот (своих не бьет)
        {
            setStyleSheet(styleSheet()+" QWidget {background: rgb(250,200,200);}");
            ui->console->setStyleSheet("QTextEdit {background: rgb(255, 225, 225);}");
            ui->connections->setStyleSheet("QListWidget{background: rgb(255, 225, 225);}");
            ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
        }
        if (normalProgram && core->getType() == 0) // обычная прога
        {

            setStyleSheet(styleSheet()+" QWidget {background: rgb(200,200,250);}");
            ui->console->setStyleSheet("QTextEdit {background: rgb(225, 225, 255);}");
            ui->connections->setStyleSheet("QListWidget{background: rgb(225, 225, 255);}");
            ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
        }

        if (troyanProgram)
        {
            setStyleSheet(styleSheet()+" QWidget {background: rgb(200,80,80);}");
            ui->console->setStyleSheet("QTextEdit { background: rgb(255, 180, 180);}");
            ui->connections->setStyleSheet("QListWidget{background: rgb(255, 180, 180);}");
            ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
        }

        if (userProgram)
        {
            setStyleSheet(styleSheet()+" QWidget#Widget {background: rgb(200,255,200); }");
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
            ui->up_i->setEnabled(1);
            ui->bar_i->setVisible(1);

            ui->up_c->setVisible(1);
            ui->up_c->setEnabled(1);
            ui->bar_c->setVisible(1);

            ui->up_d->setVisible(1);
            ui->up_d->setEnabled(1);
            ui->bar_d->setVisible(1);

            ui->find_state->setEnabled(1);
            ui->find_state->setVisible(1);

            ui->label_help->setVisible(1);
            ui->label_help_2->setVisible(1);
            ui->label_help_3->setVisible(1);
        }     
        if (wormProgram)
        {
            setStyleSheet(styleSheet()+" QWidget {background: rgb(150,120,130);}");
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

        ui->temper->setText(QString("Дружелюбность: " + QString::number(core->getConnection()->getTemper())));

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
}

void Widget::setAlive(int norm, int user, int bot)
{
    normAlive = norm;
    userAlive = user;
    botAlive = bot;

    QString str = "set: norm " + QString::number(norm) + ", user " + QString::number(user) + ", bot " + QString::number(bot);
    qDebug() << str;
    ui->console->append(str);
}

void Widget::setArgs(int argc, char *argv[])
{
    rand()%10; // костыль для рандома...
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
            if ((QString)argv[1] == "server") // троян спавнит червей
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
                if (QString(argv[3]) == "hidden")
                {
                    invisProgram = true;
                    ui->console->append("set invis");
                }

            if (argc >= 5)
                if (QString(argv[4]) == "silent")
                {
                    core->getConnection()->setSilent(1);
                    ui->console->append("set silent");
                }
        }

        else if ((QString)argv[1] == "worm") // червь - помощник трояна
        {
            wormProgram = true;
            setWindowTitle("Я - Червь");
            int type = 2;

            core = new Core(100, 1, 20, -50, 0, 0.05, type, 0);
            core->setSearch(true);
            period = 1;
            timer = startTimer(1000/period);
            timerIncrease = startTimer(1000/(50-period));

            connect(connection,
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));
        }

        else if ((QString)argv[1] == "help") // help для первого уровня
        {
            setFixedSize(200, 100);
            ui->up_c->setEnabled(true);
            ui->up_c->setVisible(true);
            ui->up_c->resize(180,80); // взята эта кнопка
            ui->up_c->move(10, 10); // чтобы не плодить лишних интерфейсов
            ui->up_c->setText("Все понятно, продолжим!"); // улучшения тут нет

            setWindowTitle("Разобрались?");
            connection = new Connection(45455, 0, -1, 0); // порт хелпера
        }

        else if ((QString)argv[1] == "timer") // счетчик для уровня с трояном
        {
            timerProgram = true;
            setFixedSize(201, 61);
            ui->myPort->setText("15:00");

            setWindowTitle("Обратный отсчет");
            setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
            connection = new Connection(45456, 0, -1, 0); // порт счетчика
            connect(connection,
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));
            timer = startTimer(1000);
            period = 900; // 900 секунд для победы
        }

        else if ((QString)argv[1] == "win") // окно победы
        {
            setFixedSize(200, 100);
            ui->up_c->setEnabled(true);
            ui->up_c->setVisible(true);
            ui->up_c->resize(180,80); // аналогично с предыдущим
            ui->up_c->move(10, 10);
            ui->up_c->setText("Открыт следующий уровень.");

            setWindowTitle("Победа!");
            connection = new Connection(45457, 0, -1, 0); // порт окна победы
        }

        else if ((QString)argv[1] == "lose") // окно поражения
        {
            setFixedSize(200, 100);
            ui->up_c->setEnabled(true);
            ui->up_c->setVisible(true);
            ui->up_c->resize(180,80); // аналогично с предыдущим
            ui->up_c->move(10, 10);
            ui->up_c->setText("Ваши программы\nбыли уничтожены.");

            setWindowTitle("Поражение :(");
            connection = new Connection(45458, 0, -1, 0);
        }

        else if ((QString)argv[1] == "info") // окно информации
        {
            setFixedSize(300, 120);
            ui->up_c->setEnabled(true);
            ui->up_c->setVisible(true);
            ui->up_c->resize(60,30); // аналогично с предыдущим
            ui->up_c->move(120, 85);
            ui->up_c->setText("Ладно");

            ui->console->setVisible(1);
            ui->console->resize(290, 75);
            ui->console->move(5,5);
            ui->console->setEnabled(1);

            if (QString(argv[2]) == "serv2")
            {
                ui->console->setText("Готовьтесь отбить нападение двух хацкеров!");
            }
            if (QString(argv[2]) == "serv3")
            {
                ui->console->setText("Босса убить нада!");
            }

            setWindowTitle("Информация");
        }

        else if ((QString)argv[1] == "educate") // режим обучения
        {
            educateProgram = true;
            userProgram = true;
            education = 1; /* этапы обучения:
                             1 - консоль
                             20 - порт, окно связей
                             30 - параметры
                             40 - улучшения параметров
                             50 - кнопки взаимодействий
                             */
            int D = 1;
            int I = 10;
            int C = 10;
            int temper = 0;
            double Ii = 0.05;
            double  Ci = 0.2;
            int type = 1;
            core = new Core(I, D, C, temper, Ii, Ci, type, 0);
            connect(core->getConnection(),
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));

            timer = startTimer(500);
            period = 1;
        }
        else if ((QString)argv[1] == "about") // режим обучения
        {
            setFixedSize(700, 600);
            ui->console->setVisible(true);
            ui->console->move(0,0);
            ui->console->resize(700, 600);
            ui->console->setStyleSheet("QTextEdit { background: rgba(0,0,0,0);"
                                       "color: lightgreen; font: 20px;}");
            setStyleSheet("QWidget#Widget { background: black; }");
            ui->console->setText("\n\nПривет!\n\n\t\tХахах!\n\n\nВыдели меня!"
                                 "\n\n\n \t\t\t\t:-)\n\nНажмите среднюю кнопку мыши для выхода.");

            ui->attack_count->setVisible(1);
            ui->attack_count->setEnabled(1);
            ui->attack_count->resize(400,20);
            ui->attack_count->setReadOnly(1);
            ui->attack_count->setStyleSheet("QLineEdit { background: rgba(0,0,0,0);"
                                            "color: #C0F0C0; font: 20px;"
                                            "border:0px;"
                                            "selection-background-color: #404040;"
                                            "selection-color: #fab700;}");
            ui->attack_count->setText("wasd3680@yandex.ru");

            maxLevel = startTimer(10);
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
            arguments << "normal" << "1"; // старт 2 прог для обучения
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
        arguments << "user" << "3"; // старт юзера
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

    if (ui->launcherTab->currentIndex() == 3) // Побег
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
            arguments.clear();

            for (int i = 0; i < 2; i++) // старт 2 ботов отложенных
            {
                arguments << "bot" << "2" << "hidden" << "silent";
                QProcess::startDetached(name, arguments);
                arguments.clear();
            }

            arguments << "normal" << "1" << "hidden" << "silent"; // норм отложенный
            QProcess::startDetached(name, arguments);
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
            setAlive(-1, 2, 1);

            arguments << "server" << "10"; // старт Сервера
            QProcess::startDetached(name, arguments);
            arguments.clear();

            arguments << "timer";
            QProcess::startDetached(name, arguments);
            arguments.clear();

            for (int i = 0; i < userAlive; i++) // старт 2 юзеров
            {
                arguments << "user" << "3";
                QProcess::startDetached(name, arguments);
                arguments.clear();
            }
        }
        else
        {
            arguments << "worm" << "0"; // старт червя
            QProcess::startDetached(name, arguments);
            arguments.clear();

            qDebug() << "bot +1";
            ui->console->append("bot +1");
            botAlive++;
        }
    }

    /*if (isHidden() == false)
        setHidden(true);*/

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
            if (education >= 30)
                education++;
        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->append("Недостаточно ресурсов.");
        }
    }
    ui->attack->setEnabled(0);
}

void Widget::on_help_clicked() // помощь пользователя выбранному порту
{
    int index = ui->connections->currentRow();
    if (index > -1) //если выбрано
    {
        int i = ui->help_count->text().toInt();
        if (core->getI() >= i &&
                core->getConnection()->getTable(index).useful >= 1 &&
                i > 0) // если достаточно
        {
            core->help(core->getConnection()->getTable(index).port, i);
            if (education >= 30)
                education++;
        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->append("Недостаточно ресурсов или пользы.");
        }

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

                        if (education >= 30)
                            education++;
                    }
                    else
                    {
                        ui->console->setTextColor(QColor(0,0,0));
                        ui->console->append("Недостаточно ресурсов.");
                    }
                }
                else
                {
                    ui->console->setTextColor(QColor(0,0,0));
                    ui->console->append("Слишком плохое отношение с "+QString::number(helper+1));
                }
            }
            else
            {
                ui->console->setTextColor(QColor(0,0,0));
                ui->console->append("Выберите различные цель и помощника.");
            }
        }
    }
    ui->request->setEnabled(0);
}


void Widget::on_connections_itemSelectionChanged() // выбран другой элемент
{
    if (core != NULL) // если класс Core создан
    {
        // запоминаем новое выделение
        core->getConnection()->setSelectedConnection(ui->connections->currentRow());
        // не работает как надо! на alpha 0.3.6 по прежнему не понимаю
    }
}



void Widget::on_find_state_toggled(bool checked) // переключен автопоиск (пользователь)
{
    core->setSearch(checked);
    if (education >= 30)
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
            ui->console->append("Недостаточно ресурсов");
        }
        ui->up_c->setEnabled(false);
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
        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->append("Недостаточно ресурсов");
        }
        ui->up_d->setEnabled(false);
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
            ui->console->append("Недостаточно ресурсов");
        }
        ui->up_i->setEnabled(false);
    }
}

void Widget::on_launcherTab_currentChanged(int index)
{
    if (launcher)
    {
        if (index == 0)
        {
        ui->console->setText("тут должно быть нормальное описание для вступления, предыстория.");
        }
        if (index == 1)
        {
            ui->console->setText("Тут че то про тестирование, обучение и отбор. Основная суть тестов - машинное обучение, ИИ и все такое");
        }
        if (index == 2)
        {
            ui->console->setText("тут 3 на 3, кооперация ИИ и около того. Бред, но что поделать");
        }
        if (index == 3)
        {
            ui->console->setText("прорыв к Серверу, 3 уровня, если действовать неправаильно, легко проиграть. Возможно нужен баланс");
        }
        if (index == 4)
        {
            ui->console->setText("Уровень с Сервером, победа или смерть. В первом случае он говорит, что ты лох и ничего не понял, и подвел свой род.\n"
                                 "В случае поражения он все равно говорит что ты лох, и удаляет твою ветвь эволюции (на компе ничего не удаляется. пока.)");
        }
        if (index == 5)
        {
            ui->console->setText("В случае победы, на серверной машине начинают безудержно развиваться и размножаться всякие проги, "
                                 "которые раньше подавлялись Сервером. Тупо угарнуть, пройти пока нельзя. Можно придумать тут миниквест, но я вообще думаю над тем,"
                                 " чтобы убрать этот режим в другую игру в другом виде. Идеи уже есть...");
        }
    }
}
