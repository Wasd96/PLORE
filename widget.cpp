#include "widget.h"
#include "ui_widget.h"
#include <QProcess>
#include <QDesktopWidget>
#include <QMessageBox>
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

    level = 0;
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

    if (t->timerId() == timer) // таймер для программ
    {

        if (core->getConnection()->getUdpSocket()->pendingDatagramSize() != -1)
            core->getConnection()->read(); // избавление от фриза сокета

        if (invisProgram && isVisible())
        {
            hide();
        }

        // обновление всех процессов
        if (userProgram)
            core->updateUser();
        if (wormProgram)
            core->updateWorm();
        if (normalProgram || troyanProgram)
            core->update();

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



        //обновление интерфейса
        double speed = (double)(10000-core->getD())/1000.0;
        ui->I->setText(QString("Доступная память: " +
                               QString::number(core->getI()) +
                               " УБ"+
                               " "+QString::number((double)core->getIi()/speed)));
        ui->D->setText(QString("Быстродействие: " +
                               QString::number(speed)+
                               " сек/оп."));
        ui->C->setText(QString("Активный ресурс: " +
                               QString::number(core->getC())+
                               " "+QString::number((double)core->getCi()/speed)));


        while(core->hasMessages())
        {
            QString str = core->getMessage(); // подсветка сообщений разного типа
            if (str.contains("Атака!"))       // по ключевым словам
                ui->console->setTextColor(QColor(255,0,0));
            else if (str.contains("Атака ->"))
                ui->console->setTextColor(QColor(140,210,0));
            else if (str.contains("Помощь ->"))
                ui->console->setTextColor(QColor(210,110,0));
            else if (str.contains("-> Помощь"))
                ui->console->setTextColor(QColor(0,255,0));
            else if (str.contains("Поиск"))
                ui->console->setTextColor(QColor(100,100,100));
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

        }


        if (userProgram)
        {
            ui->up_i->setEnabled(true);
            ui->up_d->setEnabled(true);
            ui->up_c->setEnabled(true);
            ui->attack->setEnabled(true);
            ui->help->setEnabled(true);
            ui->request->setEnabled(true);

            ui->bar_i->setMaximum(core->getINextRequire() + 5); // 100% возможность улучшения
            if (core->getC() >= core->getINextRequire()+5)
                ui->bar_i->setValue(core->getINextRequire()+5);
            else
                ui->bar_i->setValue(core->getC());

            ui->bar_d->setMaximum(core->getDNextRequire()); // 100% возможность улучшения
            if (core->getC() >= core->getDNextRequire())
                ui->bar_d->setValue(core->getDNextRequire());
            else
                ui->bar_d->setValue(core->getC());

            ui->bar_c->setMaximum(core->getCNextRequire() + 5); // 100% возможность улучшения
            if (core->getC() >= core->getCNextRequire()+5)
                ui->bar_c->setValue(core->getCNextRequire()+5);
            else
                ui->bar_c->setValue(core->getC());
        }

        if (period != 10000 - core->getD()) // если скорость обновления изменилась
        {
            period = 10000 - core->getD(); // запоминаем новую
            killTimer(timer);
            timer = startTimer(period); // запускаем ее
        }
    }

    if (t->timerId() == deathTimer)
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
        ui->console->append(QString(uiWidgets.size())+" "+QString((int)core));
        // исправить фальш текст

        if (uiWidgets.size() > 0)
        {
            disappear = disappear % uiWidgets.size();
            uiWidgets.at(disappear)->setVisible(false);
            setWindowOpacity(windowOpacity()-0.03);
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
                    killTimer(deathTimer);
                    close();
                }
            }
        }
    }
}

void Widget::paintEvent(QPaintEvent *pEv)
{
    QPainter p(this);

    if (timerProgram && isFullScreen())
    {
        if (period == -3)
            p.fillRect(0,0,width(),height(),Qt::white);
        else
            p.fillRect(0,0,width(),height(),Qt::black);
    }

    if (userProgram)
    {
        p.fillRect(0,0,width(),height(),qRgb(200,255,200));

        ui->console->setStyleSheet("QTextEdit { background: rgb(225, 255, 225);}");
    }

    if (wormProgram)
    {
        p.fillRect(0,0,width(),height(),qRgb(150,120,130));
        ui->console->setStyleSheet("QTextEdit { background: rgb(200, 180, 180);}");
        ui->connections->setStyleSheet("QListWidget{background: rgb(200, 180, 180);}");
        ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
    }

    if (troyanProgram)
    {
        p.fillRect(0,0,width(),height(),qRgb(200,80,80));
        ui->console->setStyleSheet("QTextEdit { background: rgb(255, 180, 180);}");
        ui->connections->setStyleSheet("QListWidget{background: rgb(255, 180, 180);}");
        ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
    }

    if (normalProgram && core->getType() == 2) // бот (своих не бьет)
    {
        p.fillRect(0,0,width(),height(),qRgb(250,200,200));
        ui->console->setStyleSheet("QTextEdit {background: rgb(255, 225, 225);}");
        ui->connections->setStyleSheet("QListWidget{background: rgb(255, 225, 225);}");
        ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
    }
    if (normalProgram && core->getType() == 0) // обычная прога
    {
        p.fillRect(0,0,width(),height(),qRgb(200,200,250));
        ui->console->setStyleSheet("QTextEdit {background: rgb(225, 225, 255);}");
        ui->connections->setStyleSheet("QListWidget{background: rgb(225, 225, 255);}");
        ui->connections->setSelectionMode(QAbstractItemView::NoSelection);
    }

    if (!launcher)
    {
        QPen pen;
        pen.setColor(Qt::black);
        p.setPen(pen);
        p.drawRect(0,0,width()-1,height()-1);
    }


}

void Widget::mouseMoveEvent(QMouseEvent *mEv)
{
    if (moving)
        move(mEv->globalPos().x() - movingX, mEv->globalPos().y() - movingY);
}

void Widget::mousePressEvent(QMouseEvent *mEv)
{
    if (mEv->button() == Qt::LeftButton && !launcher)
    {
        movingX = mEv->x();
        movingY = mEv->y();
        moving = true;
        return;
    }
    moving = false;

}

void Widget::mouseReleaseEvent(QMouseEvent *mEv)
{
    moving = false;
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

        if (type == 80) // спавн червя
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

            if (level == 4) // если это уровень с трояном
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
                        //QMessageBox::information(NULL, "Тандем", "Готовьтесь отбить нападение двух хацкеров!");
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
                        //QMessageBox::information(this, "Босс", "Босса убить нада!");
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
        setWindowFlags(Qt::Window);
        setFixedSize(501, 300);

        ui->start->setVisible(1);
        ui->start->setEnabled(1);
        ui->start->move(10,260);

        ui->launcherTab->setVisible(1);
        ui->launcherTab->setEnabled(1);
        ui->launcherTab->move(0,0);
        ui->launcherTab->setCurrentIndex(1);
        ui->launcherTab->setCurrentIndex(0);

        for(int i = 0; i <= maxLevel; i++)
            ui->launcherTab->setTabEnabled(i,1);

        ui->console->setVisible(1);
        ui->console->setEnabled(1);
        ui->console->resize(500, 230);
        ui->console->move(00, 20);


    }
    if (normalProgram ||
            userProgram ||
            wormProgram ||
            troyanProgram) // установка gui нормальной программы
    {
        QDesktopWidget qdw; // получение размером экрана
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
        ui->I->setText(QString("Доступная память: " +
                               QString::number(core->getI()) +
                               " УБ"));
        ui->D->setVisible(1);
        ui->D->setText(QString("Быстродействие: " +
                               QString::number((double)(10000-core->getD())/1000.0) +
                               " сек/оп."));
        ui->C->setVisible(1);
        ui->C->setText(QString("Активный ресурс: " +
                               QString::number(core->getC())));

        ui->temper->setVisible(1);
        ui->temper->setText(QString("Дружелюбность: " + QString::number(core->getTemper())));


        if (normalProgram || troyanProgram)
        {
            ui->console->resize(280, 290);
            ui->console->move(220, 10);
        }
        if (userProgram)
        {
            ui->console->resize(280, 290);
            ui->console->move(470, 10);
            ui->bar_i->setMinimum(0);
            ui->bar_i->setMaximum(core->getINextRequire() + 5); // 100% возможность улучшения
            ui->bar_i->setValue(core->getC());

            ui->bar_d->setMaximum(core->getDNextRequire()); // 100% возможность улучшения
            ui->bar_d->setValue(core->getC());

            ui->bar_c->setMaximum(core->getCNextRequire() + 5); // 100% возможность улучшения
            ui->bar_c->setValue(core->getC());

            setFixedSize(760,310);
            ui->connections->setSelectionMode(QAbstractItemView::SingleSelection);

            ui->attack->setVisible(1);
            ui->attack->setEnabled(1);
            ui->attack_count->setVisible(1);
            ui->attack_count->setVisible(1);

            ui->help->setVisible(1);
            ui->help->setEnabled(1);
            ui->help_count->setVisible(1);
            ui->help_count->setVisible(1);

            ui->request->setVisible(1);
            ui->request->setEnabled(1);
            ui->request_number->setVisible(1);
            ui->request_number->setVisible(1);

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
    ui->console->append(QString("%1").arg(argc));
    for (int i = 0; i < argc; i++)
    {
        qDebug() << argv[i];
        ui->console->append(argv[i]);
    }

    QString fullName = QString(argv[0]); // сохранение расположения (имени) программы
    int found = fullName.lastIndexOf("\\");
    if (found == -1) found = fullName.lastIndexOf("/");
    if (found != -1)
        name = fullName.right(fullName.size()-1 - found);
    else
        name = fullName;

    if (argc <= 1) //всегда минимум один аргумент - место запуска
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
                (QString)argv[1] == "troyan") // обычная программа
        {
            int power = QString(argv[2]).toInt();

            // power = 0 - скорость от 4.0 до 3.5
            // power = 1 - скорость от 3.5 до 3.0
            // power = 2 - скорость от 3.0 до 2.5
            // power = 3 - скорость от 2.5 до 2.0
            // power = 4 - скорость от 2.0 до 1.5
            // power = 5 - скорость от 1.5 до 1.0
            // power = 6 - скорость от 1.0 до 0.5
            // power > 6 - скорость от 0.5 до 0.0 (0.3)

            int D = 0;
            if (power <= 6)
            {
                D = rand()%(500)+6000+power*500;
                if (D > 9700) D = 9700;
            }
            else
                D = rand()%(400)+9400;

            int I = rand()%50+50+(power/2-1)*50;
            int C = rand()%10+5+(power/2-1)*20;
            int temper = rand()%11-5;
            int Ii = power/2;
            int Ci = 2+power/2;
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
            if ((QString)argv[1] == "troyan") // троян спавнит червей
            {
                troyanProgram = true;
                setWindowTitle("Я - Троян   }:-[");
                type = 3;
                temper = -5;
            }

            core = new Core(I, D, C, temper, Ii, Ci, type, 0);
            period = 10000 - core->getD();
            timer = startTimer(period);

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

        if ((QString)argv[1] == "worm") // червь - помощник трояна
        {
            wormProgram = true;
            setWindowTitle("Я - Червь");
            int type = 2;

            core = new Core(100, 7000, 20, -5, 0, 2, type, 0);
            core->setSearch(true);
            period = 10000 - core->getD();
            timer = startTimer(period);

            connect(connection,
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));
        }

        if ((QString)argv[1] == "help") // help для первого уровня
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

        if ((QString)argv[1] == "timer") // счетчик для уровня с трояном
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

        if ((QString)argv[1] == "win") // окно победы
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

        if ((QString)argv[1] == "lose") // окно поражения
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

        if ((QString)argv[1] == "info") // окно информации
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

        if (core == NULL)
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

    if (ui->launcherTab->currentIndex() == 0) // на равных
    {
        setAlive(3, 1, -1);


        for (int i = 0; i < normAlive; i++) // старт трех программ
        {
            arguments << "normal" << "2";

            QProcess::startDetached(name, arguments);
            arguments.clear();
        }
        arguments << "user" << "5"; // старт юзера
        QProcess::startDetached(name, arguments);
        arguments.clear();

        arguments << "help"; // старт "кнопки ОК" для обучения
        QProcess::startDetached(name, arguments);
        arguments.clear();  
    }

    if (ui->launcherTab->currentIndex() == 1) // сильнейший
    {
        setAlive(9, 1, -1);

        for (int i = 0; i < normAlive; i++) // старт девяти программ
        {
            arguments << "normal" << "3";

            QProcess::startDetached(name, arguments);
            arguments.clear();
        }
        arguments << "user" << "3"; // старт юзера
        QProcess::startDetached(name, arguments);
        arguments.clear();
    }

    if (ui->launcherTab->currentIndex() == 2) // стенка на стенку
    {
        setAlive(-1, 3, 3);

        for (int i = 0; i < botAlive; i++) // старт трех ботов
        {
            arguments << "bot" << "4";
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

    if (ui->launcherTab->currentIndex() == 3) // защита сервера
    {

        if (userAlive == 0)
        {
            setAlive(3, 1, -2);
            for (int i = 0; i < 3; i++) // старт 3 прог
            {
                arguments << "normal" << "1";
                QProcess::startDetached(name, arguments);
                arguments.clear();
            }
            arguments << "user" << "5"; // юзер
            QProcess::startDetached(name, arguments);
            arguments.clear();

            for (int i = 0; i < 2; i++) // старт 2 ботов отложенных
            {
                arguments << "bot" << "3" << "hidden" << "silent";
                QProcess::startDetached(name, arguments);
                arguments.clear();
            }

            arguments << "normal" << "3" << "hidden" << "silent"; // норм отложенный
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
    if (ui->launcherTab->currentIndex() == 4) // троян
    {
        if (userAlive == 0)
        {
            setAlive(-1, 2, 1);

            arguments << "troyan" << "6"; // старт трояна
            QProcess::startDetached(name, arguments);
            arguments.clear();

            arguments << "timer";
            QProcess::startDetached(name, arguments);
            arguments.clear();

            for (int i = 0; i < userAlive; i++) // старт 2 юзеров
            {
                arguments << "user" << "5";
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
                core->getConnection()->getTable(index).useful > 0 &&
                i > 0) // если достаточно
        {
            core->help(core->getConnection()->getTable(index).port, i);
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
            if (core->getConnection()->getTable(helper).relationship >= 4) // если достаточно
            {
                if (core->getC() >= 10)
                {
                    core->request(core->getConnection()->getTable(helper).port,
                                  core->getConnection()->getTable(index).port);
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
                ui->console->append("Слишком низкое отношение с "+QString::number(helper));
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

void Widget::on_up_i_clicked()
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

void Widget::on_launcherTab_currentChanged(int index)
{
    if (launcher)
    {
        if (index == 0)
        {
        ui->console->setText("        Ознакомительный режим. Познакомьтесь \
с управлением и оповещениями.\n\n\
1) Память - стабильность функционирования программы.\n\
2) Быстродействие - скорость выполнения одной операции.\n\
3) Ресурс - способность программы выполнять операции - \
взламывать другие, передавать им память \
или посылать запросы о поддержке.\n\n\
        Программа завершает работу после того, как у неё заканчивается \
доступная память. Атаки на программу удаляют часть памяти, \
помощь - передаёт память от одной программы к другой.\n\
        Для выбора цели действия нажмите на соответствующую строку \
в списке связей и впишите количество ресурса для операции. \
Если нужно запросить атаку, то выбранная строка будет являться \
целью, а вписанный номер - номером помощника в списке.\n\
        Дружелюбность показывает, каким действиям программа \
отдаёт приоритет. В зависимости от действий её отношения \
с другими программами может меняться.");
        }
        if (index == 1)
        {
            ui->console->setText("компиляция - выживет только 1");
        }
        if (index == 2)
        {
            ui->console->setText("Слабо тремя управлять? Враг сильнее на старте.");
        }
        if (index == 3)
        {
            ui->console->setText("Отбивай атаку за атакой!");
        }
        if (index == 4)
        {
            ui->console->setText("Попробуй убить этот троян раньше того, как он отключит компьютер!");
        }
        if (index == 5)
        {
            ui->console->setText("Дичь! Но пока ее нет, не нажимай.");
        }
    }
}
