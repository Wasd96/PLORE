#include "widget.h"
#include "ui_widget.h"
#include <QProcess>
#include <QDebug>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);


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
        if (connection->hasData())
        {
            ui->connections->clear();
            for(int i = 0; i < connection->getTableSize(); i++)
            {
                connectTable table = connection->getTable(i);
                str = QString::number(table.port) + " " +
                        QString::number(table.relationship) + " " +
                        QString::number(table.useful);
                ui->connections->appendPlainText(str);
            }
        }

        core->update();


        //обновление интерфейса
        ui->I->setText(QString("Доступная память: " +QString::number(core->getI()) +" УБ"));
        ui->D->setText(QString("Быстродействие: " +QString::number((double)(10000-core->getD())/1000.0)+" сек."));
        ui->C->setText(QString("С: " +QString::number(core->getC()) +" ?!"));


        while(core->hasMessages())
        {
            ui->console->append(core->getMessage());
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
                               " сек."));
        ui->C->setVisible(1);
        ui->C->setText(QString("С: " +
                               QString::number(core->getC()) +
                               " ?!"));

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
}

void Widget::setArgs(int argc, char *argv[])
{
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

            core = new Core(100, 9000, 0);
            period = 10000 - core->getD();
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
    close();
}

void Widget::on_send_clicked()
{
    core->send(45454, 0);
}
