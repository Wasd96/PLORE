#include "core.h"

Core::Core()
{
    connection = new Connection(rand()%200 + 50000, 0);

    In = 100;
    Dn = 9000;
    Cn = 5;

    I = (int*)malloc(sizeof(int)*In);
    D = (double*)malloc(sizeof(double)*Dn);
    C = (char*)malloc(sizeof(char)*Cn);

    temper = 0;

    Ii = 1;
    Ci = 2;

    dead = false;
}

Core::Core(int _I, int _D, int _C, int _temper, int _Ii, int _Ci)
{
    connection = new Connection(rand()%200 + 50000, _temper);

    In = _I;
    Dn = _D;
    Cn = _C;
    I = (int*)malloc(sizeof(int)*In);
    D = (double*)malloc(sizeof(double)*Dn);
    C = (char*)malloc(sizeof(char)*Cn);

    temper = _temper;

    Ii = _Ii;
    Ci = _Ci;

    dead = false;
}

void Core::send(quint16 port, int type)
{
    connection->sendData(port, type);
}

void Core::send(quint16 port, int type, int amount)
{
    connection->sendData(port, type, amount);
}

void Core::update()
{
    connection->sortTable();
    QString str; // сообщения в "консоль"
    bool op = false;
    double coeff = ((double)(Ii+Ci))/((10000.0-(double)Dn)/1000.0+1.0); // коэффициент желания апгрейда
    if (!op && Cn >= 1.3*(200.0/((10000.0-(double)Dn)/1000.0-0.2)-15) && rand()%3==0 && Dn<9500) // апгрейд быстродействия
    {
        double x = (10000.0-(double)Dn)/1000.0; // х от 9 до 0.5
        int rndInc = ((0.3*x*x+0.5*x-0.33)/6.0)*100; // случайная надбавка
        if (rndInc < 3) rndInc = 3;
        int increase = rand()%rndInc + ((0.3*x*x+0.5*x-0.33)/6.0)*900;  // идея кв. ур-ния - спасибо Полине
        if (increase == 0) increase = 1;
        int decrease = 200.0/((10000.0-(double)Dn)/1000.0-0.2)-15; // стоимость
        Cn -= decrease;
        Dn += increase;
        if (Dn > 9500) Dn = 9500;
        str = "Оптимизированы команды процессора (+"+QString::number(increase)+") за " +QString::number(decrease) +" ресурса";
        messages.append(str);
        op = true;
        if (timeToUpgrade >= (int)(50.0*coeff))
            messages.append("Все системы в норме.");
        timeToUpgrade = 0;
    }

    if (!op && Cn > 1.2*pow((Ci+1),2.5) && rand()%3==0) // увеличение прироста ресурса
    {
        int decrease = rand()%5 + pow((Ci+1),2.5);
        Cn -= decrease;
        Ci++;
        str = "Улучшены алгоритмы (прирост ресурса +1) за " + QString::number(decrease);
        messages.append(str);
        op = true;
        if (timeToUpgrade >= (int)(50.0*coeff))
            messages.append("Все системы в норме.");
        timeToUpgrade = 0;
    }

    if (!op && Cn > 1.2*((int)pow((Ii+2),3) + pow(3, Ii+1)) && rand()%4==0) // увеличение прироста памяти
    {
        int decrease = rand()%5 + (pow((Ii+2),3)) + pow(3, Ii+1);
        Cn -= decrease;
        Ii++;
        str = "Убраны утечки памяти (прирост памяти +1) за " + QString::number(decrease);
        messages.append(str);
        op = true;
        if (timeToUpgrade >= (int)(50.0*coeff))
            messages.append("Все системы в норме.");
        timeToUpgrade = 0;
    }

    if (!op && Cn > In/4 && In > 100 && timeToUpgrade < (int)(70.0*coeff)) // помощь
    {
        for (int i = 0; i < connection->getTableSize(); i++)
        {
            int relate = -connection->getTable(i).relationship;

            if (rand()%(relate+10) == 0 && connection->getTable(i).useful > 0)
            {
                int Ihelp = (double)In/10 + rand()%(1*(In/10));
                if (In - Ihelp < 100) Ihelp = In - 100;
                if (Ihelp < 10) break;
                send(connection->getTable(i).port, 4, Ihelp);
                /*if (-relate + 1 > 5)
                    connection->setRelationship(i,5);
                else
                    connection->setRelationship(i,-relate+1);*/

                connection->setUseful(i, connection->getTable(i).useful-1);
                str = "Помощь -> "+ QString::number(connection->getTable(i).port)+ " ("+QString::number(Ihelp) + " памяти).";
                messages.append(str);
                Cn -= Ihelp/10;
                In -= Ihelp;
                op = true;

                break;
            }
            if (rand()%20 == 0 && connection->getTable(i).useful == 0) // случайное повышене пользы
                connection->setUseful(i, connection->getTable(i).useful + 1);
        }
    }

    if (!op && Cn > In/(5+In/500) && In > 50 && timeToUpgrade < (int)(50.0*coeff)) // атака
    {
        for (int i = 0; i < connection->getTableSize(); i++)
        {
            int relate = connection->getTable(i).relationship;
            if (rand()%(relate+10+connection->getTable(i).useful) == 0)
            {
                int Cattack = (double)Cn/2.0 + In/200*rand()%(5*abs(relate-6));
                if (Cattack > Cn) Cattack = Cn;
                if (Cattack < 5*Ci) break;
                send(connection->getTable(i).port, 3, Cattack);

                Cn -= Cattack;
                op = true;
                str = "Атака -> " +QString::number(connection->getTable(i).port)+
                      " (" + QString::number(Cattack) + " ресурса).";
                messages.append(str);
                break;
            }
        }
    }

    if (!op && Cn > 20 && timeToUpgrade < (int)(70.0*coeff))
    {
        int enemyIndex = -1;
        int friendIndex = -1;

        for (int i = 0; i < connection->getTableSize()/2; i++)
        {
            if (connection->getTable(i).relationship == -5)
            {
                if (rand()%4 == 0)
                {
                    enemyIndex = i;
                    break;
                }
            }
            if (connection->getTable(i).relationship == -4)
            {
                if (rand()%10 == 0)
                {
                    enemyIndex = i;
                    break;
                }
            }
        }
        for (int i = connection->getTableSize()-1; i > connection->getTableSize()/2; i--)
        {
            if (connection->getTable(i).relationship == 5)
            {
                if (rand()%4 == 0)
                {
                    friendIndex = i;
                    break;
                }
            }
            if (connection->getTable(i).relationship == 4)
            {
                if (rand()%10 == 0)
                {
                    friendIndex = i;
                    break;
                }
            }
        }

        if (friendIndex > -1 && enemyIndex > -1) // найдены враг и друг
        {
            Cn -= 10;
            send(connection->getTable(friendIndex).port,5,connection->getTable(enemyIndex).port);
            str = "Запрос боевой помощи -> "+
                    QString::number(connection->getTable(friendIndex).port) +
                    " в борьбе с " +
                    QString::number(connection->getTable(enemyIndex).port);
            messages.append(str);
        }
    }





    if (!op && Cn >= connection->getTableSize()*10+10 && timeToUpgrade < (int)(90.0*coeff)) // поиск связей
    {
        quint16 port = rand()%200 + 50000;
        while (port == connection->getPort()) // нет смысла себе писать
            port = rand()%200 + 50000;
        send(port, 0); // отправка поискового сообщения
        Cn -= 1; // стоимость поиска
        str = "Поиск -> " + QString::number(port); // отчет
        messages.append(str);
        op = true;
    }

    for (int i = 0; i < connection->getTableSize(); i++) // проверка связи, не зависит от параметров
    {
        if (connection->getTable(i).lostSignal >= 3) // если три раза не ответил
        {
            messages.append("Соединение с " + QString::number(connection->getTable(i).port) + " было потеряно.");
            connection->deleteTable(i); // удаление записи
        }
        else
        {
            connection->setLostSignal(i, connection->getTable(i).lostSignal + 1); // пропущенные +1
            send(connection->getTable(i).port, 1); // проверка связи
        }
    }




    while(connection->hasData()) // считывание входящих (отчетов)
    {
        str = connection->getData();
        QStringList strList = str.split(" ");
        if (strList.size() > 1)
        {
            if (strList.at(1) == "3") // если это атака
            {
                //str = "на момент атаки: C" + QString::number(Cn) +" I" +QString::number(In);
                //messages.append(str);
                quint16 port = strList.at(0).toInt(); //порт атакующего
                double amount = strList.at(2).toInt(); // сколько пришло ресурса
                //double decrease = (amount/(double)(amount+Cn))*amount; // кажется, делает бои вечными
                double decrease = amount;
                if (decrease/3.0 > Cn) // если ресурса меньше чем треть атаки
                {
                    str = QString::number(port) +
                            "-> Атака! (-" +
                            QString::number((int)Cn) +
                            " ресурса, -" +
                            QString::number((int)(decrease - Cn)) +
                            " памяти).";
                    In -= decrease - Cn; // то память получает остаток урона
                    Cn = 0;
                }
                else
                {
                    str = QString::number(port) +
                            "-> Атака! (-" +
                            QString::number((int)(decrease/3.0)) +
                            " ресурса, -" +
                            QString::number((int)(decrease*2.0/3.0)) +
                            " памяти).";
                    Cn -= decrease/3.0;
                    In -= decrease*2.0/3.0;
                }
                messages.append(str);
            }
            else if (strList.at(1) == "4") // если это помощь
            {
                quint16 port = strList.at(0).toInt(); //порт помощника
                double amount = strList.at(2).toInt(); // сколько пришло помощи

                In += amount;

                str = QString::number(port) +
                        "-> Помощь (+" +
                        QString::number((int)amount) +
                        " памяти).";
                messages.append(str);
            }
            else if (strList.at(1) == "5") // если это помощь в бою
            {
                quint16 senderPort = strList.at(0).toInt(); //порт просящего
                quint16 targetPort = strList.at(2).toInt(); // порт цели
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
                    connectTable table = {targetPort, -5, 0, 0};
                    connection->createTable(table);
                    send(targetPort, 0);
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
                    if (connection->getTable(senderIndex).relationship > connection->getTable(targetIndex).relationship
                            && connection->getTable(senderIndex).useful > 0) // УБРАТЬ
                    {
                        connection->setRelationship(targetIndex, -5);
                        connection->setUseful(targetIndex, 0);
                        connection->setUseful(senderIndex, connection->getTable(senderIndex).useful - 1);
                        str = QString::number(senderPort) +
                              "-> просит помощи в борьбе с " +
                              QString::number(targetPort);
                        messages.append(str);
                        if (Cn > In/10 && In > 50) // если может помочь
                        {
                            str = "Атака -> " +QString::number(targetPort)+
                                  " (" + QString::number(Cn/3) + " ресурса).";
                            messages.append(str);

                            send(targetPort, 3, Cn/3); // атака
                            send(senderPort, 6); // отчет об успешной атаке
                            Cn -= Cn/3;
                        }
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

    if (In <= 0)
    {
        dead = true; // смерть
        free(I);
        free(D);
        free(C);
        free(connection);
        return;
    }


    if (timeToUpgrade == (int)(50.0*coeff))
        messages.append("Желательна оптимизация: атака неактивна.");
    if (timeToUpgrade == (int)(70.0*coeff))
        messages.append("Требуется оптимизация: помощь неактивна.");
    if (timeToUpgrade == (int)(90.0*coeff))
        messages.append("Необходима оптимизация: поиск портов отключен.");
    timeToUpgrade++;

    In += Ii; // прирост памяти
    if (Cn < In) // предел ресурса
    {
        Cn += Ci; // ресурса
    }


    I = (int*)realloc(I, In*sizeof(int)); //перерасчет
    D = (double*)(realloc(D, Dn*sizeof(double)));
    C = (char*)realloc(C, Cn*sizeof(char));
}

