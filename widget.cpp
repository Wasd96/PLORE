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
    troyanProgram = false;
    wormProgram = false;
    exploreProgram = false;
    timerProgram = false;

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
    if (t->timerId() == timer && timerProgram)
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
                            QProcess::startDetached("PLORE.exe", args);

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

        // обновление всех процессов
        if (userProgram)
            core->updateUser();
        if (wormProgram)
            core->updateWorm();
        if (normalProgram)
            core->update();

        if (core->getDead()) // смерть программы естественным путем
        {
            delete core;
            killTimer(timer);
            close();
            return;
        }


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
            else if (str.contains("Скомпилирован"))
                ui->console->setTextColor(QColor(126,58,105));
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



}

void Widget::died(int type)
{
    if (launcher)
    {
        if (type == 0) // умер норм прог
        {
            normAlive--;
            qDebug() << normAlive << userAlive << botAlive;
        }
        if (type == 1) // умер юзер
        {
            userAlive--;
            qDebug() << normAlive << userAlive << botAlive;
        }
        if (type == 2) // умер бот
        {
            botAlive--;
            qDebug() << normAlive << userAlive << botAlive;
        }
        if (type == 3) // умер троян
        {
            botAlive--;
            qDebug() << normAlive << userAlive << botAlive;
        }
        if (type == 80) // спавн червя
        {
            on_start_clicked();
        }
        if (type == 90) // конец уровня
        {
            for (int i = 0; i < 200; i++)
            {
                connection->sendData(50000+i, 88); // всем умереть
            }

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
                    file.write(&save, 1); // считывание макс. уровня
                }
                else
                {
                    file.open(QIODevice::WriteOnly); // создаем файл, если его не было
                }
                file.close();
            }

            setHidden(false);
            raise();

            for(int i = 0; i <= maxLevel; i++)
                ui->launcherTab->setTabEnabled(i,1);

            userAlive = 0;
            botAlive = 0;
            normAlive = 0;
            return;
        }

        if (userAlive == 0 || botAlive == 0 || normAlive == 0)
        {
            if (level == 4 && userAlive != 0) // если это уровень с трояном
            {
                connection->sendData(45456, 88);
            }
            if (userAlive == 0)
            {
                qDebug() << "Game Over";

                QStringList args;
                args << "lose";
                QProcess::startDetached("PLORE.exe", args);

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
                        //QMessageBox::information(NULL, "Тандем", "Готовьтесь отбить нападение двух хацкеров!");
                        on_start_clicked();
                        return;
                    }
                    else if (botAlive == 0)
                    {
                        //QMessageBox::information(this, "Босс", "Босса убить нада!");
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
    if (normalProgram || userProgram || wormProgram) // установка gui нормальной программы
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
            ui->console->resize(200,130);
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
                (QString)argv[1] == "bot" ||
                (QString)argv[1] == "troyan") // обычная программа
        {
            int power = QString(argv[2]).toInt();

            // power = 0 - скорость от 6.0 до 5.2
            // power = 1 - скорость от 5.5 до 4.7
            // power = 2 - скорость от 5.0 до 4.2
            // power = 3 - скорость от 4.5 до 3.7
            // power = 4 - скорость от 4.0 до 3.2
            // power = 5 - скорость от 3.5 до 2.7
            // power = 6 - скорость от 3.0 до 2.2
            // power = 7 - скорость от 2.5 до 1.7
            // power = 8 - скорость от 2.0 до 1.2
            // power = 9 - скорость от 1.5 до 0.7
            int D = rand()%(800)+4000+power*500;

            int I = rand()%50+50+power/3*50;
            int C = rand()%10+5+power/3*50;
            int temper = rand()%11-5;
            int Ii = power/3;
            int Ci = 2+power/3;
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
            if ((QString)argv[1] == "troyan") // троян спавнит ботов
            {
                normalProgram = true;
                setWindowTitle("Я - Троян   }:-[");
                type = 3;
                temper = -5;
            }
            core = new Core(I, D, C, temper, Ii, Ci, type);
            period = 10000 - core->getD();
            timer = startTimer(period);

            connect(core->getConnection(),
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));
        }

        if ((QString)argv[1] == "worm") // червь - помощник трояна
        {
            wormProgram = true;
            setWindowTitle("Я - Червь");
            int type = 2;

            core = new Core(100, 7000, 20, -5, 0, 2, type);
            core->setSearch(true);
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

        if ((QString)argv[1] == "timer") // счетчик для уровня с трояном
        {
            timerProgram = true;
            setFixedSize(201, 61);
            ui->myPort->setText("25:00");

            setWindowTitle("Обратный отсчет");
            setWindowFlags(windowFlags() ^ Qt::WindowStaysOnTopHint);
            connection = new Connection(45456, 0, -1); // порт счетчика
            connect(connection,
                    SIGNAL(died(int)),
                    this,
                    SLOT(died(int)));
            timer = startTimer(1000);
            period = 1500; // 1500 секунд для победы
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

        if ((QString)argv[1] == "lose") // окно поражения
        {
            setFixedSize(200, 100);
            ui->up_c->setEnabled(true);
            ui->up_c->setVisible(true);
            ui->up_c->resize(180,80); // аналогично с предыдущим
            ui->up_c->move(10, 10);
            ui->up_c->setText("Ваши программы\nбыли уничтожены.");

            setWindowTitle("Поражение :(");
            connection = new Connection(45455, 0, -1);
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
            arguments << "normal" << "3";

            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();
        }
        arguments << "user" << "5"; // старт юзера
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
            arguments << "normal" << "4";

            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();
        }
        arguments << "user" << "4"; // старт юзера
        QProcess::startDetached("PLORE.exe", arguments);
        arguments.clear();

        botAlive = 9;
        userAlive = 1;
        botAlive = -1;
    }

    if (ui->launcherTab->currentIndex() == 2) // стенка на стенку
    {

        for (int i = 0; i < 3; i++) // старт трех ботов
        {
            arguments << "bot" << "5";
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();
        }
        for (int i = 0; i < 3; i++) // старт 3 юзеров
        {
            arguments << "user" << "3";
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
            for (int i = 0; i < 4; i++) // старт 4 прог
            {
                arguments << "normal" << "3";
                QProcess::startDetached("PLORE.exe", arguments);
                arguments.clear();
            }
            arguments << "user" << "5"; // юзер
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();

            botAlive = -2;
            userAlive = 1;
            normAlive = 4;
        }
        else if (normAlive == 0)
        {
            for (int i = 0; i < 2; i++) // старт двух ботов
            {
                arguments << "bot" << "7";
                QProcess::startDetached("PLORE.exe", arguments);
                arguments.clear();
            }

            botAlive = 2;
            normAlive = -1;
        }
        else if (botAlive == 0)
        {
            arguments << "normal" << "9";
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();
            normAlive = 1;
            botAlive = -1;
        }
    }
    if (ui->launcherTab->currentIndex() == 4) // троян
    {
        if (userAlive == 0)
        {
            arguments << "troyan" << "8"; // старт трояна
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();

            arguments << "timer";
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();

            for (int i = 0; i < 2; i++) // старт 2 юзеров
            {
                arguments << "user" << "4";
                QProcess::startDetached("PLORE.exe", arguments);
                arguments.clear();
            }
            botAlive = 1;
            userAlive = 2;
            normAlive = -1;
        }
        else
        {
            arguments << "worm" << "0"; // старт червя
            QProcess::startDetached("PLORE.exe", arguments);
            arguments.clear();

            botAlive++;
        }
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
