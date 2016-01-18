#include "widget.h"
#include "ui_widget.h"
#include <QProcess>
#include <QDebug>


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    connection = new Connection();
    connection->send(0);

    // инициализация типов программ
    launcher = false;
    normalProgram = false;
    invisProgram = false;
    clanProgram = false;
    exploreProgram = false;


    //инициализация GUI
    disableGUI();
}

void Widget::initGUI()
{
    if (launcher) // установка gui лаунчера
    {
        ui->start->setVisible(1);
        ui->start->setEnabled(1);

    }
    if (normalProgram) // установка gui нормальной программы
    {
        ui->console->setVisible(1);
        ui->console->setEnabled(1);

    }
}

void Widget::disableGUI()
{
    ui->console->setEnabled(0);
    ui->console->setVisible(0);

    ui->start->setEnabled(0);
    ui->start->setVisible(0);
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
            setWindowTitle("Я - Макс");
        }
    }


    initGUI();
}



Widget::~Widget()
{
    delete ui;
}

void Widget::on_start_clicked()
{
    QStringList arguments;
    arguments << "n";
    qDebug() << QProcess::startDetached("PLORE.exe", arguments);
}

void Widget::on_pushButton_clicked()
{
    close();
}
