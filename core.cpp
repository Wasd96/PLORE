#include "core.h"

Core::Core()
{
    srand(time(NULL));
    connection = new Connection(rand()%100 + 45000);
}

Core::Core(int _I, int _D, int _C)
{
    srand(time(NULL));
    connection = new Connection(rand()%100 + 45000);

    I = (int*)malloc(sizeof(int)*_I);
    D = (double*)malloc(sizeof(double)*_D);
    C = (char*)malloc(sizeof(char)*_C);

    In = _I;
    Dn = _D;
    Cn = _C;
}

void Core::send(quint16 port, int type)
{
    connection->send(port, type);
}

void Core::update()
{
    quint16 port = rand()%100 + 45000;
    while (port == connection->getPort())
        port = rand()%100 + 45000;
    send(port, 0); // поиск по рандомному порту
    QString str;
    str = "Sending to port " + QString::number(port);
    messages.append(str);

    while(connection->hasData())
    {
        messages.append(connection->getData());
    }

    In ++;
    I = (int*)realloc(I, In*sizeof(int));
}

