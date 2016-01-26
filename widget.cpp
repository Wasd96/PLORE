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
    userProgram = false;
    normalProgram = false;
    invisProgram = false;
    clanProgram = false;
    exploreProgram = false;


    //инициализация GUI
    disableGUI();


    setFixedSize(400,300);
    move(100,100);
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint );
}

void Widget::timerEvent(QTimerEvent *t) // таймер, частота работы проги
{
    if (t->timerId() == timer)
    {
        // обновления

        QString str;
        Connection* connection = core->getConnection();
        ui->connections->clear(); // заполнение таблицы связей
        for(int i = 0; i < connection->getTableSize(); i++)
        {
            connectTable table = connection->getTable(i);
            str = QString::number(i+1) + ") " +
                    QString::number(table.port%1000) + " отношение: " +
                    QString::number(table.relationship) + " польза: " +
                    QString::number(table.useful) + " юзер: "  +
                    QString::number(table.type);
            ui->connections->addItem(str);
        }
        //выделяем предыдущее значение
        ui->connections->setCurrentRow(core->getConnection()->getSelectedConnection());

        if (userProgram)
            core->updateUser();
        else
            core->update(); // обновление всех процессов


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
            else
                ui->console->setTextColor(QColor(0,0,0));
            ui->console->append(str);

        }

        if (core->getDead())
        {
            close();
        }

        if (userProgram)
        {
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
}

void Widget::initGUI()
{

    if (launcher) // установка gui лаунчера
    {


        ui->start->setVisible(1);
        ui->start->setEnabled(1);
        ui->start->move(10,260);

        ui->Choose->setVisible(1);
        ui->Choose->setEnabled(1);
        ui->Choose->move(290, 260);

        ui->launcherTab->setVisible(1);
        ui->launcherTab->setEnabled(1);
        ui->launcherTab->move(0,0);
        setWindowFlags(Qt::Window);

    }
    if (normalProgram || userProgram) // установка gui нормальной программы
    {
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
        ui->temper->setText(QString("Настроение (заменить): " + QString::number(core->getTemper())));


        if (normalProgram)
        {
            ui->console->move(220, 10);
        }
        if (userProgram)
        {
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

    ui->Choose->setEnabled(0);
    ui->Choose->setVisible(0);

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

            int D = rand()%2000+7000;
            if (D < 4000) // в зависимости от скорости даются бонусы
            {
                int I = rand()%50+100;
                int C = rand()%10+10;
                int temper = rand()%6+0;
                core = new Core(I, D, C, temper, 2, 3, 0);
            }
            else if (D < 6500)
            {
                int I = rand()%50+80;
                int C = rand()%10+7;
                int temper = rand()%9-3;
                core = new Core(I, D, C, temper, 2, 2, 0);
            }
            else
            {
                int I = rand()%50+50;
                int C = rand()%10+5;
                int temper = rand()%11-5;
                core = new Core(I, D, C, temper, 1, 2, 0);
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

        if ((QString)argv[1] == "u") // пользователь!
        {
            userProgram = true;
            setWindowTitle("Я - Человек");

            int D = rand()%2000+7000;
            if (D < 4000) // в зависимости от скорости даются бонусы
            {
                int I = rand()%50+100;
                int C = rand()%10+10;
                int temper = rand()%6+0;
                core = new Core(I, D, C, temper, 2, 3, 1);
            }
            else if (D < 6500)
            {
                int I = rand()%50+80;
                int C = rand()%10+7;
                int temper = rand()%9-3;
                core = new Core(I, D, C, temper, 2, 2, 1);
            }
            else
            {
                int I = rand()%50+50;
                int C = rand()%10+5;
                int temper = rand()%11-5;
                core = new Core(I, D, C, temper, 1, 2, 1);
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
    setHidden(true);
    QStringList arguments;

    if (ui->launcherTab->currentIndex() == 0)
    {
        if (ui->Choose->currentText() == "норм")
        {
            arguments << "n";
        }
        if (ui->Choose->currentText() == "юзер")
        {
            arguments << "u";
        }
    }

    qDebug() << QProcess::startDetached("PLORE.exe", arguments);
}

void Widget::on_pushButton_clicked()
{
    //delete core;
    close();
}

void Widget::on_send_clicked()
{
    core->send(45454, 0);
}

void Widget::on_attack_clicked() // атака пользователя на выбранный порт
{
    int index = ui->connections->currentRow();

    if (index > -1) //если выбрано
    {
        int c = ui->attack_count->text().toInt();
        if (core->getC() >= c) // если достаточно
        {
            core->attack(core->getConnection()->getTable(index).port, c);

        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->append("Недостаточно ресурсов.");
        }
    }
}

void Widget::on_help_clicked() // помощь пользователя выбранному порту
{
    int index = ui->connections->currentRow();
    if (index > -1) //если выбрано
    {
        int i = ui->help_count->text().toInt();
        if (core->getI() >= i && core->getConnection()->getTable(index).useful > 0) // если достаточно
        {
            core->help(core->getConnection()->getTable(index).port, i);
        }
        else
        {
            ui->console->setTextColor(QColor(0,0,0));
            ui->console->append("Недостаточно ресурсов или пользы.");
        }

    }
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
}


void Widget::on_connections_itemSelectionChanged() // выбран другой элемент
{
    if (core != NULL) // если класс Core создан
    {
        // запоминаем новое выделение
        core->getConnection()->setSelectedConnection(ui->connections->currentRow());
    }
}



void Widget::on_find_state_toggled(bool checked) // отключен автопоиск (пользователь)
{
    core->setSearch(checked);
}

void Widget::on_up_c_clicked()
{
    if ((double)ui->bar_c->value()/(double)ui->bar_c->maximum() == 1)
    {
        core->upgradeC();
    }
    else
    {
        ui->console->setTextColor(QColor(0,0,0));
        ui->console->append("Недостаточно ресурсов");
    }
}

void Widget::on_up_d_clicked()
{
    if ((double)ui->bar_d->value()/(double)ui->bar_d->maximum() == 1)
    {
        core->upgradeD();
    }
    else
    {
        ui->console->setTextColor(QColor(0,0,0));
        ui->console->append("Недостаточно ресурсов");
    }
}

void Widget::on_up_i_clicked()
{
    if ((double)ui->bar_i->value()/(double)ui->bar_i->maximum() == 1)
    {
        core->upgradeI();
    }
    else
    {
        ui->console->setTextColor(QColor(0,0,0));
        ui->console->append("Недостаточно ресурсов");
    }
}
