#include "widget.h"
#include "ui_widget.h"
#include <QProcess>
#include <QDebug>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    srand(time(NULL));

    // инициализация типов программ
    launcher = false;
    normalProgram = false;
    invisProgram = false;
    clanProgram = false;
    exploreProgram = false;


    //инициализация GUI
    disableGUI();


    setFixedSize(400,300);
    move(100,100);
    //setWindowOpacity(0.9);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint );
}

void Widget::timerEvent(QTimerEvent *t) // таймер, частота работы проги
{
    if (t->timerId() == timer)
    {
        // обновления

        QString str;
        Connection* connection = core->getConnection();
        ui->connections->clear();
        for(int i = 0; i < connection->getTableSize(); i++)
        {
            connectTable table = connection->getTable(i);
            str = QString::number(table.port) + " отношение: " +
                    QString::number(table.relationship) + " польза: " +
                    QString::number(table.useful);
            ui->connections->appendPlainText(str);
        }

        core->update();


        //обновление интерфейса
        ui->I->setText(QString("Доступная память: " +
                               QString::number(core->getI()) +
                               " УБ"));
        ui->D->setText(QString("Быстродействие: " +
                               QString::number((double)(10000-core->getD())/1000.0)+
                               " сек/оп."));
        ui->C->setText(QString("Активный ресурс: " +
                               QString::number(core->getC())));


        while(core->hasMessages())
        {
            QString str = core->getMessage();
            if (str.contains("Атака!"))
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
            else
                ui->console->setTextColor(QColor(0,0,0));
            ui->console->append(str);

        }

        if (core->getDead())
        {
            close();
        }

        if (period != 10000 - core->getD())
        {
            period = 10000 - core->getD();
            killTimer(timer);
            timer = startTimer(period);
        }
    }
}

void Widget::initGUI()
{
    if (launcher) // установка gui лаунчера
    {
        ui->start->setVisible(1);
        ui->start->setEnabled(1);
        ui->start->move(10,260);

        ui->launcherTab->setVisible(1);
        ui->launcherTab->setEnabled(1);
        ui->launcherTab->move(0,0);
        setWindowFlags(Qt::Window);

    }
    if (normalProgram) // установка gui нормальной программы
    {
        setFixedSize(500, 300);

        ui->console->setVisible(1);
        ui->console->setEnabled(1);
        ui->connections->setEnabled(1);
        ui->connections->setVisible(1);


        ui->myPort->setVisible(1);
        ui->myPort->setText(QString("Порт: "+
                                    QString::number(core->getConnection()->getPort())));
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
        ui->temper->setText(QString("Настроение (заменить): " + QString::number(core->getTemper())));
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

    ui->pushButton->setVisible(0);
    ui->pushButton->setEnabled(0);

    ui->send->setVisible(0);
    ui->send->setEnabled(0);

    ui->myPort->setVisible(0);
    ui->I->setVisible(0);
    ui->D->setVisible(0);
    ui->C->setVisible(0);
    ui->temper->setVisible(0);
}

void Widget::setArgs(int argc, char *argv[])
{
    rand()%10;
    qDebug() << argc;
    ui->console->append(QString("%1").arg(argc));
    for (int i = 0; i < argc; i++)
    {
        qDebug() << argv[i];
        ui->console->append(argv[i]);
    }
    if (argc <= 1) //всегда минимум один аргумент - место запуска
    {
        launcher = true; // значит это лаунчер
        setWindowTitle("Лаунчер");
    }
    else
    {
        if ((QString)argv[1] == "n") // обычная программа
        {
            normalProgram = true;
            setWindowTitle("Я - Программа");

            int D = rand()%7000+2000;
            if (D < 4000)
            {
                int I = rand()%50+100;
                int C = rand()%10+10;
                int temper = rand()%3+3;
                core = new Core(I, D, C, temper, 2, 3);
            }
            else if (D < 6500)
            {
                int I = rand()%50+80;
                int C = rand()%10+7;
                int temper = rand()%8-2;
                core = new Core(I, D, C, temper, 2, 2);
            }
            else
            {
                int I = rand()%50+50;
                int C = rand()%10+5;
                int temper = rand()%11-5;
                core = new Core(I, D, C, temper, 1, 2);
            }
            /*core = new Core(rand()%50+50,
                            rand()%7000+2000,
                            rand()%10+5,
                            rand()%11-5,
                            1,
                            2);*/
            period = 10000 - core->getD();
            //ui->console->append(QString::number(core->getD()));
            timer = startTimer(period);
        }
    }


    initGUI();
}



Widget::~Widget()
{
    delete ui;
}

void Widget::on_start_clicked() // старт игры
{
    QStringList arguments;
    if (ui->launcherTab->currentIndex() == 0)
    {
        arguments << "n";
    }
    qDebug() << QProcess::startDetached("PLORE.exe", arguments);
}

void Widget::on_pushButton_clicked()
{
    delete core;
    close();
}

void Widget::on_send_clicked()
{
    core->send(45454, 0);
}
