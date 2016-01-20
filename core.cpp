#include "core.h"

Core::Core()
{
    connection = new Connection(rand()%100 + 45000);

    In = 100;
    Dn = 9000;
    Cn = 5;

    I = (int*)malloc(sizeof(int)*In);
    D = (double*)malloc(sizeof(double)*Dn);
    C = (char*)malloc(sizeof(char)*Cn);

    temper = 1;

    Ii = 1;
    Ci = 2;
}

Core::Core(int _I, int _D, int _C, int _temper, int _Ii, int _Ci)
{
    connection = new Connection(rand()%100 + 45000);

    In = _I;
    Dn = _D;
    Cn = _C;
    I = (int*)malloc(sizeof(int)*In);
    D = (double*)malloc(sizeof(double)*Dn);
    C = (char*)malloc(sizeof(char)*Cn);

    temper = _temper;

    Ii = _Ii;
    Ci = _Ci;
}

void Core::send(quint16 port, int type)
{
    connection->send(port, type);
}

void Core::update()
{
    QString str; // сообщения в "консоль"
    bool op = false;


    if (!op && Cn >= 1.3*(200.0/((10000.0-(double)Dn)/1000.0-0.2)-15) && rand()%4==0 && Dn<9500) // апгрейд быстродействия
    {
        double x = (10000.0-(double)Dn)/1000.0; // х от 9 до 0.5
        int rndInc = ((0.3*x*x+0.5*x-0.33)/6.0)*100; // случайная надбавка
        if (rndInc < 3) rndInc = 3;
        int increase = rand()%rndInc + ((0.3*x*x+0.5*x-0.33)/6.0)*900;  // идея кв. ур-ния - спасибо Полине
        int decrease = 200.0/((10000.0-(double)Dn)/1000.0-0.2)-15; // стоимость
        Cn -= decrease;
        Dn += increase;
        if (Dn > 9500) Dn = 9500;
        str = "Оптимизированы команды процессора (+"+QString::number(increase)+") за " +QString::number(decrease) +" ресурса";
        messages.append(str);
        op = true;
    }

    if (!op && Cn > 1.2*(pow((Ci+1),3)) && rand()%3==0) // увеличение прироста ресурса
    {
        Cn -= pow((Ci+1),3);
        Ci++;
        str = "Улучшены алгоритмы (прирост ресурса +1) за " + QString::number(pow(Ci,3));
        messages.append(str);
        op = true;
    }

    if (!op && Cn > 1.2*((int)pow((Ii+2),3)) && rand()%4==0) // увеличение прироста памяти
    {
        Cn -= (int)pow((Ii+2),3);
        Ii++;
        str = "Убраны утечки памяти (прирост памяти +1) за " + QString::number((int)pow(Ii+1,3));
        messages.append(str);
        op = true;
    }




    if (!op && Cn >= connection->getTableSize()*10+10) // поиск связей
    {
        quint16 port = rand()%100 + 45000;
        while (port == connection->getPort())
            port = rand()%100 + 45000;
        send(port, 0);
        Cn -= 1; // стоимость поиска
        str = "Поиск -> " + QString::number(port);
        messages.append(str);
        op = true;
    }




    while(connection->hasData()) // считывание входящих
    {
        messages.append(connection->getData());
    }


    In += Ii; // прирост памяти
    if (Cn < In/3)
    {
        Cn += Ci; // ресурса
    }


    I = (int*)realloc(I, In*sizeof(int)); //перерасчет
    D = (double*)(realloc(D, Dn*sizeof(double)));
    C = (char*)realloc(C, Cn*sizeof(char));
}

