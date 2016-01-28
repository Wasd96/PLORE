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
    clanProgram = false;
    exploreProgram = false;

    level = 0;
    maxLevel = 0;

    userAlive = 0;
    botAlive = 0;
    normAlive = 0;

    //инициализация GUI
    disableGUI();


    setFixedSize(200,100);
    move(200,200);
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
            delete core;
            close();
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
}

void Widget::died(int type)
{
    if (launcher)
    {
        if (type == 0) // умер норм прог
        {
            normAlive--;
            qDebug() << userAlive << botAlive;
        }
        if (type == 1) // умер юзер
        {
            userAlive--;
            qDebug() << userAlive << botAlive;
        }
        if (type == 2) // умер бот
        {
            botAlive--;
            qDebug() << userAlive << botAlive;
        }
        if (type == 90) // конец уровня
        {
            for (int i = 0; i < 200; i++)
            {
                connection->sendData(50000+i, 88); // всем умереть
            }
            if (maxLevel == 0)
                maxLevel = 1;
            ui->launcherTab->setCurrentIndex(maxLevel);
            setHidden(false);
            raise();

            for(int i = 0; i <= maxLevel; i++)
                ui->launcherTab->setTabEnabled(i,1);

            QFile file("save"); // файл сохранения
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) // попытка открыть
            {
                char save = maxLevel;
                file.write(&save, 1); // считывание макс. уровня
            }
            else
            {
                file.open(QIODevice::WriteOnly); // создаем файл, если его не было
            }
            file.close();

            userAlive = 0;
            botAlive = 0;
            normAlive = 0;
            return;
        }

        if (userAlive == 0 || botAlive == 0 || normAlive == 0)
        {
            if (userAlive == 0)
            {
                qDebug() << "Game Over";

                userAlive = -1;
                botAlive = -1;
                normAlive = -1;
            }
            else
            {
                if (level == 3)
                {
                    if (normAlive == 0 && botAlive == -2) // первый этап
                    {
                        QMessageBox::information(this, "Тандем", "Готовьтесь отбить нападение двух хацкеров!");
                        on_start_clicked();
                        return;
                    }
                    else if (botAlive == 0)
                    {
                        QMessageBox::information(this, "Босс", "Босса убить нада!");
                        on_start_clicked();
                        return;
                    }
                }
                if (maxLevel > 0)
                {
                    if (maxLevel < 5)
                        maxLevel++;
                    QStringList args;
                    args << "win";
                    QProcess::startDetached("PLORE.exe", args);
                    qDebug() << "You win";
                }

                userAlive = -1;
                botAlive = -1;
                normAlive = -1;
            }
        }
    }
    else
    {
        //delete Core;
        close();
    }
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
        ui->console->resize(480, 210);
        ui->console->move(10, 30);


    }
    if (normalProgram || userProgram) // установка gui нормальной программы
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


        if (normalProgram)
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
        connection = new Connection(45454, 0, -1); // личный порт лаунчера
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

    }
    else
    {
        if ((QString)argv[1] == "normal" ||
                (QString)argv[1] == "user" ||
                (QString)argv[1] == "bot") // обычная программа
        {
            int power = QString(argv[2]).toInt();

            int D = rand()%(7000-power*1000)+2500+power*1000;
            int I, C, temper, Ii, Ci;
            if (D < 4000) // в зависимости от скорости даются бонусы
            {
                I = rand()%50+100+power*50;
                C = rand()%10+10+power*50;
                temper = rand()%6+0;
                Ii = 2+power;
                Ci = 3+power;
            }
            else if (D < 6500)
            {
                I = rand()%50+80+power*50;
                C = rand()%10+7+power*50;
                temper = rand()%9-3;
                Ii = 2+power;
                Ci = 2+power;
            }
            else
            {
                I = rand()%50+50+power*50;
                C = rand()%10+5+power*50;
                temper = rand()%11-5;
                Ii = 1+power;
                Ci = 3+power;
            }
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
            core = new Core(I, D, C, temper, Ii, Ci, type);
            period = 10000 - core->getD();
            timer = startTimer(period);

            connect(core->getConnection(),
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
            connection = new Connection(45455, 0, -1); // порт хелпера

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
            connection = new Connection(45455, 0, -1); // порт окна победы (в принципе, такой же у хелпа)
                                                       // роли это не играет - принимать ничего не надо
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
    level = ui->launcherTab->currentIndex();

    if (ui->launcherTab->currentIndex() == 0) // на равных
    {

        for (int i = 0; i < 3; i++) // старт трех программ
        {
            arguments << "normal" << "0";

            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();
        }
        arguments << "user" << "0"; // старт юзера
        QProcess::startDetached("PLORE.exe", arguments);
        arguments.clear();

        arguments << "help"; // старт "кнопки ОК" для обучения
        QProcess::startDetached("PLORE.exe", arguments);
        arguments.clear();

        normAlive = 3;
        userAlive = 1;
        botAlive = -1;
    }

    if (ui->launcherTab->currentIndex() == 1) // сильнейший
    {

        for (int i = 0; i < 9; i++) // старт девяти программ
        {
            arguments << "normal" << "0";

            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();
        }
        arguments << "user" << "0"; // старт юзера
        QProcess::startDetached("PLORE.exe", arguments);
        arguments.clear();

        botAlive = 9;
        userAlive = 1;
        botAlive = -1;
    }

    if (ui->launcherTab->currentIndex() == 2) // стенка на стенку
    {

        for (int i = 0; i < 3; i++) // старт трех ботов (+1)
        {
            arguments << "bot" << "1";
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();
        }
        for (int i = 0; i < 3; i++) // старт 3 юзеров
        {
            arguments << "user" << "0";
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();
        }
        botAlive = 3;
        userAlive = 3;
        normAlive = -1;
    }
    if (ui->launcherTab->currentIndex() == 3) // защита сервера
    {
        if (userAlive == 0)
        {
            for (int i = 0; i < 5; i++) // старт пяти прог
            {
                arguments << "normal" << "0";
                QProcess::startDetached("PLORE.exe", arguments);
                arguments.clear();
            }
            arguments << "user" << "1"; // юзер (+1)
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();

            botAlive = -2;
            userAlive = 1;
            normAlive = 5;
        }
        else if (normAlive == 0)
        {
            for (int i = 0; i < 2; i++) // старт двух ботов (+2)
            {
                arguments << "bot" << "2";
                QProcess::startDetached("PLORE.exe", arguments);
                arguments.clear();
            }

            botAlive = 2;
        }
        else if (botAlive == 0)
        {
            arguments << "normal" << "5";
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();
            normAlive = 1;
        }
    }

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



void Widget::on_find_state_toggled(bool checked) // отключен автопоиск (пользователь)
{
    core->setSearch(checked);
}

void Widget::on_up_c_clicked()
{

    if (size().height() == 100) // если это кнопка конца уровня
    {
        connection->sendData(45454, 90); // конец уровня
        close();
    }
    else
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
        ui->console->setText("Ознакомительный режим. Осмотритесь, \
познакомьтесь с управлением и оповещениями.\n\n\
1) Память - это состояние программы, насколько стабильно она \
функционирует в операционной системе.\n\
2) Быстродействие - показатель, сколько секунд требуется программе \
на выполнение одной операции.\n\
3) Ресурс - способность программы выполнять операции - \
взламывать другие, передавать им память \
или посылать запросы о поддержке.\n\n\
Программа завершается после того, как у нее заканчивается \
доступная память. Атаки на программу удаляют часть памяти, \
помощь же в свою очередь передает память от одной программы \
к другой.\nДружелюбность показывает как программа должна \
взаимодействовать с другими - атаковать, помогать или все вместе. \
В зависимости от действий  ее отношения с другими может меняться.");
        }
        if (index == 1)
        {
            ui->console->setText("Хех дерись! 9 на одного.");
        }
        if (index == 2)
        {
            ui->console->setText("Слабо тремя управлять? Враг сильнее на старте.");
        }
        if (index == 3)
        {
            ui->console->setText("Рано еще, не нажимай.");
        }
    }
}
