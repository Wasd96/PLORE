#include "core.h"

Core::Core()
{
    connection = new Connection(rand()%200 + 50000, 0, 0, 0);

    In = 100;
    Dn = 1;
    Cn = 50;

    I = (int*)malloc(sizeof(int)*In);
    D = (double*)malloc(sizeof(double)*Dn);
    C = (char*)malloc(sizeof(char)*Cn);

    type = 0;

    Ii = 0.05;
    Ci = 0.1;

    dead = false;
    search = true;
    searchTimeOut = 0;

    timeToUpgrade = 0;
    coeff = -1;

    requestAttack = -1;
    requestAttackSender = -1;

    for (int i =0; i < 9; i++)
        modules[i] = true;

    nextRecount();
}

Core::Core(double _I, int _D, double _C, int _temper, double _Ii, double _Ci, int _type, bool _silent)
{
    connection = new Connection(rand()%200 + 50000, _temper, _type, _silent);

    In = _I;
    Dn = _D;
    Cn = _C;
    I = (int*)malloc(sizeof(int)*In);
    D = (double*)malloc(sizeof(double)*Dn);
    C = (char*)malloc(sizeof(char)*Cn);

    type = _type;

    Ii = _Ii;
    Ci = _Ci;

    dead = false;
    search = true;
    searchTimeOut = 0;

    timeToUpgrade = 0;
    coeff = 1;

    requestAttack = -1;
    requestAttackSender = -1;

    for (int i =0; i < 9; i++)
        modules[i] = true;

    nextRecount();
}

Core::~Core()
{
    delete connection;
    delete I;
    delete D;
    delete C;
}

void Core::send(quint16 port, int _type)
{
    connection->sendData(port, _type);
}

void Core::send(quint16 port, int _type, int amount)
{
    connection->sendData(port, _type, amount);
}

void Core::attack(quint16 port, int amount)
{
    QString str;
    send(port, 3, amount);
    Cn -= amount;
    str = "Атака " +QString::number(port%1000)+
          " мощностью в " + QString::number(amount) + " ресурса";
    if (port == requestAttack && type == 1)
    {
        send(requestAttackSender, 6);
        requestAttack = -1;
    }
    messages.append(str);
}

void Core::help(quint16 port, int amount)
{
    QString str;
    send(port, 4, amount);
    int index = -1;
    for (int i = 0; i < connection->getTableSize(); i++)
    {
        if (connection->getTable(i).port == port)
            index = i; // номер цели в таблице
    }
    if (index == -1) return;
    connection->setUseful(index, connection->getTable(index).useful-5);
    Cn -= amount/5;
    In -= amount;
    str = "Отправлено " + QString::number(amount) + " памяти к "+ QString::number(connection->getTable(index).port%1000);
    messages.append(str);

}

void Core::request(quint16 portHelper, quint16 portEnemy)
{
    QString str;
    Cn -= 10;
    send(portHelper, 5, portEnemy);
    int index = -1;
    for (int i = 0; i < connection->getTableSize(); i++)
    {
        if (connection->getTable(i).port == portHelper)
            index = i; // номер помощника в таблице
    }
    connection->setUseful(index, connection->getTable(index).useful-10);
    str = "Запрос боевой помощи у "+
            QString::number(portHelper%1000) +
            " в борьбе с " +
            QString::number(portEnemy%1000);
    messages.append(str);
}

void Core::upgradeI()
{
    int decrease = rand()%5 + getINextRequire();
    Cn -= decrease;
    Ii += 0.05;
    messages.append("Убраны утечки памяти (выделение памяти +1 за " + QString::number(decrease) +")");
    if (timeToUpgrade >= (int)(coeff))
        messages.append("Оптимизация не требуется.");
    timeToUpgrade = 0;
    nextRecount();
}

void Core::upgradeD()
{
    //int increase = rand()%rndInc + ((x*x+0.5*x+0.5)/6.0)*500;  // идея кв. ур-ния - спасибо Полине (история после 0.9.7)
    int decrease = getDNextRequire(); // стоимость
    Cn -= decrease;
    Dn += 1;
    messages.append("Оптимизированы команды процессора (+"+QString::number(1)+" за " +QString::number(decrease) +")");
    if (timeToUpgrade >= (int)(coeff))
        messages.append("Оптимизация не требуется.");
    timeToUpgrade = 0;
    nextRecount();
}

void Core::upgradeC()
{
    int decrease = rand()%5 + getCNextRequire();
    Cn -= decrease;
    Ci += 0.1;
    messages.append("Улучшены алгоритмы (генерация ресурса +2 за " + QString::number(decrease) + ")");
    if (timeToUpgrade >= (int)(coeff))
        messages.append("Оптимизация не требуется.");
    timeToUpgrade = 0;
    nextRecount();
}

void Core::deathRecountRealloc()
{
    if (In < 1)
    {
        In = 0;
        dead = true; // смерть
        return;
    }
    else if (dead && In > 0) // спасение
    {
        dead = false;
    }

    if (modules[6] == true) In += Ii; // прирост памяти, если модуль в порядке
    if (modules[7] == true) Cn += Ci; // прирост ресурса, если модуль в порядке

    I = (int*)realloc(I, In*sizeof(int)); // перевыделение памяти
    D = (double*)(realloc(D, Dn*sizeof(double))); // по факту незаметна разница
    C = (char*)realloc(C, Cn*sizeof(char)); // но ради чистоты идеи
}

void Core::findConnections()
{
    if (connection->getSilent() == 0 && searchTimeOut >= 5)
    {
        quint16 port;
        int foundTable;
        bool ready = false;
        while (!ready)
        {
            port = rand()%200;
            foundTable = getConnection()->getFoundTableAt(port);
            if (rand()%(foundTable+2) == 0)
            {
                getConnection()->setFoundTableAtInc(port);
                ready = true;
            }
            else
            {
                port = rand()%200;
            }
            if (port == connection->getPort()-50000)  // нет смысла себе писать
                ready = false;
        }
        send(port+50000, 0); // отправка поискового сообщения
        Cn -= 1; // стоимость поиска
        messages.append("Поиск...");

        searchTimeOut = 0;
    }
    searchTimeOut++;
}

void Core::connectionSupport() // проверка связи с существующими соединениями
{
    for (int i = 0; i < connection->getTableSize(); i++)
    {
        if (connection->getTable(i).lostSignal >= 3) // если три раза не ответил
        {
            messages.append("Соединение с " + QString::number(connection->getTable(i).port%1000) + " было потеряно.");
            connection->deleteTable(i); // удаление записи
            break;
        }
        else
        {
            connection->setLostSignal(i, connection->getTable(i).lostSignal + 1); // пропущенные +1
            send(connection->getTable(i).port, 1); // проверка связи
        }
    }
}

void Core::operateDataFromConnection()
{
    QString str;
    while(connection->hasData()) // считывание входящих (отчетов)
    {
        str = connection->getData();
        QStringList strList = str.split(" ");
        if (strList.size() > 1)
        {
            if (strList.at(2) == "3") // если это атака
            {
                quint16 port = strList.at(0).toInt(); //порт атакующего
                double amount = strList.at(3).toInt(); // сколько пришло ресурса

                int decreaseI = 0;
                int decreaseC = 0;
                double cof = 0.1+(rand()%40)/100.0;
                if (amount*cof > Cn) // если ресурса меньше чем треть атаки
                {
                    decreaseC = Cn;
                    decreaseI = amount - decreaseC; // то память получает остаток урона
                }
                else
                {     
                    decreaseC = amount*cof;
                    decreaseI = amount*(1-cof);
                }

                int percentage = (double)(decreaseC)/Cn*100;
                if (percentage > 100) percentage = 100;
                for (int i = 0; i < 9; i++)
                {
                    if (rand()%(105-percentage) < 3)
                        modules[i] = false;
                }

                Cn -= decreaseC;
                In -= decreaseI;
                str = QString::number(port%1000) +
                        " -> Атака! Потеряно " +
                        QString::number(decreaseC) +
                        " ресурса, " +
                        QString::number(decreaseI) +
                        " памяти";
                messages.append(str);
            }

            else if (strList.at(2) == "4") // если это помощь
            {
                quint16 port = strList.at(0).toInt(); //порт помощника
                double amount = strList.at(3).toInt(); // сколько пришло помощи

                In += amount;

                int i;
                for (i = 0; i < connection->getTableSize(); i++)
                {
                    if (connection->getTable(i).port == port)
                        break; // номер цели в таблице
                }

                int incr = (amount/In)*70;
                if (incr == 0) incr = 1;
                if (incr > 30) incr = 30;
                connection->setUseful(i, connection->getTable(i).useful+incr);
                if (connection->getTable(i).useful > 50) connection->setUseful(i, 50);
                connection->setRelationship(i, connection->getTable(i).relationship+incr*2/3); // улучшение отношений
                if (connection->getTable(i).relationship > 50) connection->setRelationship(i, 50);;

                str = QString::number(port%1000) +
                        " -> Помощь. Принято " +
                        QString::number((int)amount) +
                        " памяти";
                messages.append(str);
            }

            else if (strList.at(2) == "5") // если это запрос помощи в бою
            {
                quint16 senderPort = strList.at(0).toInt(); //порт просящего
                quint16 targetPort = strList.at(3).toInt(); // порт цели
                int targetType = strList.at(4).toInt(); // тип цели
                int targetIndex = -1;
                int senderIndex = -1;
                for (int i = 0; i < connection->getTableSize(); i++)
                {
                    if (connection->getTable(i).port == targetPort)
                        targetIndex = i; // номер цели в таблице
                    if (connection->getTable(i).port == senderPort)
                        senderIndex = i; // номер отправителя в таблице
                }
                if (targetIndex == -1) // это новая связь
                {
                    connectTable table = {targetPort, -40, 0, 0, targetType, 0, false};
                    connection->createTable(table); // создание такой связь
                    send(targetPort, 0); // добавление себя в список цели
                    for (int i = 0; i < connection->getTableSize(); i++)
                    {
                        if (connection->getTable(i).port == targetPort)
                            targetIndex = i; // номер цели в таблице
                        if (connection->getTable(i).port == senderPort)
                            senderIndex = i; // номер отправителя в таблице
                    }

                }
                if (targetIndex != -1) // если такая связь есть
                {
                    if (type != 1) // если это не человек
                    {
                        str = QString::number(senderPort%1000) +
                              " -> просит помощи в борьбе с " +
                              QString::number(targetPort%1000);
                        messages.append(str);

                        if (connection->getTable(senderIndex).relationship > connection->getTable(targetIndex).relationship
                                && connection->getTable(senderIndex).useful > 0)
                        {
                            connection->setRelationship(targetIndex, -40);
                            connection->setUseful(targetIndex, 0);
                            connection->setUseful(senderIndex, connection->getTable(senderIndex).useful - 10);

                            if (Cn > In/10 && In > 50) // если может помочь
                            {
                                attack(targetPort, Cn/3); // атака
                                send(senderPort, 6); // отчет об успешной атаке
                            }
                            else
                            {
                                messages.append("Запрос отклонен - недостаточно ресурсов для атаки");
                            }
                        }
                        else
                        {
                            messages.append("Запрос отклонен - плохое отношение с отправителем");
                        }
                    }
                    else // если человек
                    {
                        str = QString::number(senderPort%1000) +
                              " -> просит помощи в борьбе с " +
                              QString::number(targetPort%1000);
                        requestAttack = targetPort;
                        requestAttackSender = senderPort;
                        messages.append(str);
                    }
                }
            }
            else if (strList.at(2) != "0" &&
                     strList.at(2) != "1" &&
                     strList.at(2) != "2" &&
                     strList.at(2) != "6" &&
                     strList.at(2) != "88" &&
                     strList.at(2) != "70" &&
                     strList.at(2) != "71")
            {
                messages.append(str);
            }
        }
        else /*if (strList.at(2) != "0" &&
                 strList.at(2) != "1" &&
                 strList.at(2) != "2" &&
                 strList.at(2) != "6" &&
                 strList.at(2) != "88" &&
                 strList.at(2) != "70" &&
                 strList.at(2) != "71")*/
        {
            messages.append(str);
        }
    }
}

void Core::coeffRecount()
{
    coeff = (20*Ii+20*Ci+Dn)*20.0; // коэффициент желания апгрейда
}

void Core::nextRecount()
{
    INextRequired = pow((30*Ii+2),3)*2; // 30!
    DNextRequired = pow(Dn, 1.8)*5 + 10;
    CNextRequired = pow((10*Ci+1),2)*6; // 10!
}

void Core::update()
{
    connection->sortTable();
    bool op = false;
    coeffRecount();

    if (!op && Cn >= 10+getDNextRequire() && rand()%10==0) // апгрейд быстродействия
    {
        upgradeD();
        op = true;
    }
    if (!op && Cn > 10+getCNextRequire() && rand()%10==0) // увеличение прироста ресурса
    {
        upgradeC();
        op = true;
    }
    if (!op && Cn > 10+getINextRequire() && rand()%10==0) // увеличение прироста памяти
    {
        upgradeI();
        op = true;
    }

    if (!op && Cn > In/20
            && In > 100
            && timeToUpgrade < (int)(1.4*coeff)
            && rand()%10 == 0
            && modules[1] == true) // помощь
    {
        for (int i = 0; i < connection->getTableSize(); i++)
        {
            connectTable link = connection->getTable(i);

            if (rand()%(-link.relationship+60) == 0 && link.useful >= 1 &&
                    ((type == 2 && link.type == 2) || type != 2) &&
                    type != 3)
            {
                int Ihelp = (double)In/20 + rand()%(int)(In/20);
                if (In - Ihelp < 100) Ihelp = In - 100;
                if (Ihelp < 10) break;

                help(connection->getTable(i).port, Ihelp);
                op = true;
                break;
            }
            if (rand()%20 == 0 && connection->getTable(i).useful == 0) // случайное повышене пользы
                connection->setUseful(i, 1);
        }
    }

    if (!op && Cn > In/10
            && In > 40
            && timeToUpgrade < (int)(1.0*coeff)
            && rand()%10 == 0
            && modules[0] == true) // атака
    {
        for (int i = 0; i < connection->getTableSize(); i++)
        {
            connectTable link = connection->getTable(i);
            int relate = link.relationship;
            int targetType = link.type;
            if (((type == 2 && targetType != 2 && targetType != 3) || type != 2) &&  // правила выбора цели
                    ((type == 3 && targetType != 2 && targetType != 3) || type != 3))
            {
                if (rand()%(relate/2+26+link.useful/2) == 0) // шанс ударить
                {
                    int Cattack = Cn/1.5 + rand()%(int)(Cn/4);
                    if (Cattack > Cn) Cattack = Cn;
                    if (Cattack < 150*Ci) break;
                    attack(link.port, Cattack);
                    op = true;
                    break;
                }
            }
        }
    }

    if (!op && Cn > 20
            && timeToUpgrade < (int)(1.4*coeff)
            && type != 3 // сервер не помогает
            && rand()%20 == 0
            && modules[2] == true) // запрос боевой помощи
    {
        if (connection->getTableSize() >= 2)
        {
            if (connection->getTable(0).relationship <= -20
                    && connection->getTable(connection->getTableSize()-1).relationship >= 20
                    && connection->getTable(connection->getTableSize()-1).useful >= 10)
            {
                request(connection->getTable(connection->getTableSize()-1).port, connection->getTable(0).port);
            }
        }


    }


    if (!op && Cn >= connection->getTableSize()*50+50
            && timeToUpgrade < (int)(1.7*coeff)
            && modules[5] == true)    // поиск связей
    {
        findConnections();
        op = true;
    }

    if (type == 3) // сервер
    {
        if (rand()%600 == 0 && In > 225 && Cn > 100)
        {
            In -= 25;
            Cn -= 50;
            send(45454, 80);
            messages.append("Скомпилирован новый червь за 25 памяти, 50 ресурса");
        }
    }

    connectionSupport(); // поддержка связи
    operateDataFromConnection(); // обработка сообщений

    if (timeToUpgrade == (int)(1.0*coeff))
        messages.append("Желательна оптимизация: атака неактивна");
    if (timeToUpgrade == (int)(1.4*coeff))
        messages.append("Требуется оптимизация: помощь неактивна");
    if (timeToUpgrade == (int)(1.7*coeff))
        messages.append("Необходима оптимизация: поиск портов отключен");
    timeToUpgrade++;
}

void Core::updateUser() // пользовательский апдейт
{
    connection->sortTable();

    if (search && Cn > 1 && modules[5] == true) // поиск
    {
        findConnections();
    }

    connectionSupport(); // поддержка связи
    operateDataFromConnection(); // обработка сообщений
}

void Core::updateWorm()
{
    connection->sortTable();
    bool op = false;

    if (!op && modules[1] == true) // помощь
    {
        for (int i = 0; i < connection->getTableSize(); i++)
        {
            if (rand()%5 == 0 && connection->getTable(i).type == 3 && In > 20)
            {
                int Ihelp = 10 + rand()%10;
                connection->setUseful(i, 2);
                help(connection->getTable(i).port, Ihelp);

                op = true;
                break;
            }
        }
    }

    if (!op && Cn > 10 && modules[0] == true) // атака
    {
        for (int i = 0; i < connection->getTableSize(); i++)
        {
            int targetType = connection->getTable(i).type;
            if (rand()%8 == 0 && targetType != 2 && targetType != 3)
            {
                attack(connection->getTable(i).port, Cn/2);
                op = true;
                break;
            }
        }
    }

    if (Cn > 1 && modules[5] == true) // поиск
    {
        findConnections();
    }

    connectionSupport(); // поддержка связи
    operateDataFromConnection(); // обработка сообщений
}
