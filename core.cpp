#include "core.h"

Core::Core()
{
    connection = new Connection(rand()%200 + 50000, 0 ,0, 0);

    In = 100;
    Dn = 9000;
    Cn = 5;

    I = (int*)malloc(sizeof(int)*In);
    D = (double*)malloc(sizeof(double)*Dn);
    C = (char*)malloc(sizeof(char)*Cn);

    temper = 0;
    type = 0;

    Ii = 1;
    Ci = 2;

    dead = false;
    search = true;

    timeToUpgrade = 0;
}

Core::Core(int _I, int _D, int _C, int _temper, int _Ii, int _Ci, int _type, bool _silent)
{
    connection = new Connection(rand()%200 + 50000, _temper, _type, _silent);

    In = _I;
    Dn = _D;
    Cn = _C;
    I = (int*)malloc(sizeof(int)*In);
    D = (double*)malloc(sizeof(double)*Dn);
    C = (char*)malloc(sizeof(char)*Cn);

    temper = _temper;
    type = _type;

    Ii = _Ii;
    Ci = _Ci;

    dead = false;
    search = true;

    timeToUpgrade = 0;
}

Core::~Core()
{
    delete connection;

    free(I);
    free(D);
    free(C);
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
    str = "Атака -> " +QString::number(port%1000)+
          " (" + QString::number(amount) + " ресурса).";
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
    connection->setUseful(index, connection->getTable(index).useful-1);
    Cn -= amount/10;
    In -= amount;
    str = "Помощь -> "+ QString::number(connection->getTable(index).port%1000)+ " ("+QString::number(amount) + " памяти).";
    messages.append(str);

}

void Core::request(quint16 portHelper, quint16 portEnemy)
{
    QString str;
    Cn -= 10;
    send(portHelper, 5, portEnemy);
    str = "Запрос боевой помощи -> "+
            QString::number(portHelper%1000) +
            " в борьбе с " +
            QString::number(portEnemy%1000);
    messages.append(str);
}

void Core::upgradeI()
{
    QString str;
    double coeff = ((double)(Ii+Ci))/((10000.0-(double)Dn)/1000.0+1.0);
    int decrease = rand()%5 + getINextRequire();
    Cn -= decrease;
    Ii++;
    str = "Убраны утечки памяти (прирост памяти +1) за " + QString::number(decrease) +" ресурса";
    messages.append(str);
    if (timeToUpgrade >= (int)(50.0*coeff))
        messages.append("Все системы в норме.");
    timeToUpgrade = 0;
}

void Core::upgradeD()
{
    QString str;
    double coeff = ((double)(Ii+Ci))/((10000.0-(double)Dn)/1000.0+1.0); // для обновлений
    double x = (10000.0-(double)Dn)/1000.0; // х от 4 до 0.2
    int rndInc = ((0.3*x*x+0.5*x-0.33)/6.0)*100; // случайная надбавка
    if (rndInc < 3) rndInc = 3;
    int increase = rand()%rndInc + ((x*x+0.5*x+0.5)/6.0)*500;  // идея кв. ур-ния - спасибо Полине
    if (increase == 0) increase = 1;
    int decrease = getDNextRequire(); // стоимость
    Cn -= decrease;
    Dn += increase;
    if (Dn > 9800) Dn = 9800;
    str = "Оптимизированы команды процессора (+"+QString::number(increase)+") за " +QString::number(decrease) +" ресурса";
    messages.append(str);
    if (timeToUpgrade >= (int)(50.0*coeff))
        messages.append("Все системы в норме.");
    timeToUpgrade = 0;
}

void Core::upgradeC()
{
    QString str;
    double coeff = ((double)(Ii+Ci))/((10000.0-(double)Dn)/1000.0+1.0);
    int decrease = rand()%5 + getCNextRequire();
    Cn -= decrease;
    Ci++;
    str = "Улучшены алгоритмы (прирост ресурса +1) за " + QString::number(decrease) +" ресурса";
    messages.append(str);
    if (timeToUpgrade >= (int)(50.0*coeff))
        messages.append("Все системы в норме.");
    timeToUpgrade = 0;
}

void Core::deathRecountRealloc()
{
    if (In <= 0)
    {
        messages.append("gonna die");
        if (dead)
            return;
        else
        {
            dead = true; // смерть
            send(45454, 1, type); // сообщение лаунчеру о своей смерти
            messages.append("died");
            return;
        }
    }

    In += Ii; // прирост памяти
    if (Cn < In) // предел ресурса
    {
        Cn += Ci; // прирост ресурса
        if (Cn > In) // специально для Димы
            Cn = In;
    }


    I = (int*)realloc(I, In*sizeof(int)); // перевыделение памяти
    D = (double*)(realloc(D, Dn*sizeof(double))); // по факту незаметна разница
    C = (char*)realloc(C, Cn*sizeof(char)); // но ради чистоты идеи
}

void Core::findConnections()
{
    if (connection->getSilent() == 0)
    {
        QString str;
        quint16 port = rand()%200 + 50000;
        while (port == connection->getPort()) // нет смысла себе писать
            port = rand()%200 + 50000;
        send(port, 0); // отправка поискового сообщения
        Cn -= 1; // стоимость поиска
        str = "Поиск -> " + QString::number(port%1000); // отчет
        messages.append(str);
    }
}

int Core::getINextRequire()
{
    return (int)pow((Ii+2),3) + pow(3, Ii+1);
}

int Core::getDNextRequire()
{
    return 200.0/((10000.0-(double)Dn)/1000.0-0.18)-50;
}

int Core::getCNextRequire()
{
    return pow((Ci+1),2.5);
}

void Core::connectionSupport() // проверка связи с существующими соединениями
{
    for (int i = 0; i < connection->getTableSize(); i++)
    {
        if (connection->getTable(i).lostSignal >= 3) // если три раза не ответил
        {
            messages.append("Соединение с " + QString::number(connection->getTable(i).port%1000) + " было потеряно.");
            connection->deleteTable(i); // удаление записи
            if (connection->getTableSize() > 0)
                connection->setSelectedConnection(connection->getTableSize()/2);
            else
                connection->setSelectedConnection(-1);
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
            if (strList.at(1) == "3") // если это атака
            {
                quint16 port = strList.at(0).toInt(); //порт атакующего
                double amount = strList.at(2).toInt(); // сколько пришло ресурса

                int decreaseI = 0;
                int decreaseC = 0;
                if (amount/3.0 > Cn) // если ресурса меньше чем треть атаки
                {
                    decreaseC = Cn;
                    decreaseI = amount - decreaseC; // то память получает остаток урона
                }
                else
                {     
                    decreaseC = amount/3.0;
                    decreaseI = amount*2.0/3.0;
                }
                Cn -= decreaseC;
                In -= decreaseI;
                str = QString::number(port%1000) +
                        "-> Атака! (-" +
                        QString::number(decreaseC) +
                        " ресурса, -" +
                        QString::number(decreaseI) +
                        " памяти).";
                messages.append(str);
            }

            else if (strList.at(1) == "4") // если это помощь
            {
                quint16 port = strList.at(0).toInt(); //порт помощника
                double amount = strList.at(2).toInt(); // сколько пришло помощи

                In += amount;

                str = QString::number(port%1000) +
                        "-> Помощь (+" +
                        QString::number((int)amount) +
                        " памяти).";
                messages.append(str);
            }

            else if (strList.at(1) == "5") // если это помощь в бою
            {
                quint16 senderPort = strList.at(0).toInt(); //порт просящего
                quint16 targetPort = strList.at(2).toInt(); // порт цели
                int targetType = strList.at(3).toInt(); // тип цели
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
                    connectTable table = {targetPort, -5, 0, 0, targetType};
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

                    if (type != 1) // если это бот
                    {
                        str = QString::number(senderPort%1000) +
                              "-> просит помощи в борьбе с " +
                              QString::number(targetPort%1000);
                        messages.append(str);

                        if (connection->getTable(senderIndex).relationship > connection->getTable(targetIndex).relationship
                                && connection->getTable(senderIndex).useful > 0)
                        {
                            connection->setRelationship(targetIndex, -5);
                            connection->setUseful(targetIndex, 0);
                            connection->setUseful(senderIndex, connection->getTable(senderIndex).useful - 1);

                            if (Cn > In/10 && In > 50) // если может помочь
                            {
                                attack(targetPort, Cn/3); // атака
                                send(senderPort, 6); // отчет об успешной атаке
                            }
                            else
                            {
                                messages.append("Запрос отклонен - Недостаточно ресурсов для атаки.");
                            }
                        }
                        else
                        {
                            messages.append("Запрос отклонен - низкое отношение с отправителем.");
                        }
                    }
                    else // если человек
                    {
                        str = QString::number(senderPort%1000) +
                              "-> просит помощи в борьбе с " +
                              QString::number(targetPort%1000);
                        messages.append(str);
                    }

                }
            }
            else
            {
                messages.append(str);
            }
        }
        else
        {
            messages.append(str);
        }
    }
}

void Core::update()
{
    connection->sortTable();
    QString str; // сообщения в "консоль"
    bool op = false;
    double coeff = ((double)(Ii+Ci))/((10000.0-(double)Dn)/1000.0+1.0); // коэффициент желания апгрейда


    if (!op && Cn >= 1.3*getDNextRequire() && rand()%3==0 && Dn<9500) // апгрейд быстродействия
    {
        upgradeD();
        op = true;
    }
    if (!op && Cn > 1.2*getCNextRequire() && rand()%3==0) // увеличение прироста ресурса
    {
        upgradeC();
        op = true;
    }
    if (!op && Cn > 1.2*getINextRequire() && rand()%4==0) // увеличение прироста памяти
    {
        upgradeI();
        op = true;
    }

    if (!op && Cn > In/4
            && In > 100
            && timeToUpgrade < (int)(70.0*coeff)) // помощь
    {
        for (int i = 0; i < connection->getTableSize(); i++)
        {
            int relate = -connection->getTable(i).relationship;

            if (rand()%(relate+10) == 0 &&
                    connection->getTable(i).useful > 0 &&
                    ((type == 2 && connection->getTable(i).type == 2) || type != 2) &&
                    type != 3)
            {
                int Ihelp = (double)In/10 + rand()%(1*(In/10));
                if (In - Ihelp < 100) Ihelp = In - 100;
                if (Ihelp < 10) break;

                help(connection->getTable(i).port, Ihelp);

                op = true;
                break;
            }
            if (rand()%20 == 0 && connection->getTable(i).useful == 0) // случайное повышене пользы
                connection->setUseful(i, connection->getTable(i).useful + 1);
        }
    }

    if (!op && Cn > In/(5+In/500)
            && In > 50
            && timeToUpgrade < (int)(50.0*coeff)) // атака
    {
        for (int i = 0; i < connection->getTableSize(); i++)
        {
            int relate = connection->getTable(i).relationship;
            int targetType = connection->getTable(i).type;
            if (((type == 2 && targetType != 2 && targetType != 3) || type != 2) &&
                    ((type == 3 && targetType != 2 && targetType != 3) || type != 3))
            {
                if (rand()%(relate+10+connection->getTable(i).useful) == 0)
                {
                    int Cattack = (double)Cn/2.0 + In/200*rand()%(5*abs(relate-6));
                    if (Cattack > Cn) Cattack = Cn;
                    if (Cattack < 5*Ci) break;
                    attack(connection->getTable(i).port, Cattack);
                    op = true;
                    break;
                }
            }
        }
    }

    if (!op && Cn > 20
            && timeToUpgrade < (int)(70.0*coeff)
            && type != 3) // запрос боевой помощи
    {
        int enemyIndex = -1;
        int friendIndex = -1;

        for (int i = 0; i < connection->getTableSize()/2; i++) // выбор врага
            if (connection->getTable(i).relationship <= -4)
                if (rand()%(4+6*(5-abs(connection->getTable(i).relationship))) == 0)
                {        // ^ 4 - если отношение -5, 10 - если отн. -4
                    enemyIndex = i;
                    break;
                }
        for (int i = connection->getTableSize()-1; i > connection->getTableSize()/2; i--) // выбор помощника
            if (connection->getTable(i).relationship >= 4)
                if (rand()%(4+6*(5-connection->getTable(i).relationship)) == 0)
                {        // ^ 4 - если отношение 5, 10 - если отн. 4
                    friendIndex = i;
                    break;
                }

        if (friendIndex > -1 && enemyIndex > -1) // найдены враг и друг
        {
            request(connection->getTable(friendIndex).port, connection->getTable(enemyIndex).port);
        }
    }





    if (!op && Cn >= connection->getTableSize()*10+10
            && timeToUpgrade < (int)(90.0*coeff))    // поиск связей
    {
        findConnections();
        op = true;
    }

    if (type == 3) // троян
    {
        if (rand()%60 == 0 && In > 25 && Cn > 50)
        {
            In -= 25;
            Cn -= 50;
            send(45454, 80);
            messages.append("Скомпилирован новый червь! (-25 памяти, -50 ресурса)");
        }
    }

    connectionSupport(); // поддержка связи


    operateDataFromConnection(); // обработка сообщений


    if (timeToUpgrade == (int)(50.0*coeff))
        messages.append("Желательна оптимизация: атака неактивна.");
    if (timeToUpgrade == (int)(70.0*coeff))
        messages.append("Требуется оптимизация: помощь неактивна.");
    if (timeToUpgrade == (int)(90.0*coeff))
        messages.append("Необходима оптимизация: поиск портов отключен.");
    timeToUpgrade++;


    deathRecountRealloc(); // обработка смерти, подсчетов, перевыделений
}

void Core::updateUser() // пользовательский апдейт
{
    connection->sortTable();
    QString str; // сообщения в "консоль"



    if (search && Cn > 1) // поиск
    {
        findConnections();
    }


    connectionSupport(); // поддержка связи

    operateDataFromConnection(); // обработка сообщений


    deathRecountRealloc(); // обработка смерти, подсчетов, перевыделений
}

void Core::updateWorm()
{
    connection->sortTable();
    QString str; // сообщения в "консоль"
    bool op = false;


    if (!op) // помощь
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

    if (!op && Cn > 10) // атака
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


    if (Cn > 1) // поиск
    {
        findConnections();
    }

    connectionSupport(); // поддержка связи

    operateDataFromConnection(); // обработка сообщений

    deathRecountRealloc(); // обработка смерти, подсчетов, перевыделений
}
