#ifndef CORE_H
#define CORE_H

#include "connection.h"


class Core
{
private:

    Connection* connection;

    // характеристики
    int* I;
    double* D;
    char* C;

    int In, Dn, Cn; // кол-во параметров каждого типа

    int temper; // настроение, характер, -5 - злой, +5 - добрый

    int Ii, Ci; // прирост памяти и активного ресурса



    QList<QString> messages;

public:
    Core();
    Core(int _I, int _D, int _C, int _temper, int _Ii, int _Ci);

    void send(quint16 port, int type);
    void update();
    bool hasMessages() { return (bool)messages.size(); }
    Connection* getConnection() { return connection;}

    int getI() { return In; }
    int getD() { return Dn; }
    int getC() { return Cn; }
    QString getMessage() { return messages.takeLast(); }
};

#endif // CORE_H
