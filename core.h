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

    bool dead; // мертв, пора удалять
    bool search; // флаг автопоиск
    bool type; // тип проги 0 - бот 1 - юзер

    int timeToUpgrade; // желание улучшить параметры (от застоя)

    QList<QString> messages;

public:
    Core();
    Core(int _I, int _D, int _C, int _temper, int _Ii, int _Ci, int _type);

    void update();
    void updateUser();
    void connectionSupport();
    void operateDataFromConnection();

    void send(quint16 port, int type);
    void send(quint16 port, int type, int amount);
    void setSearch(bool _search) { search = _search; }
    void setType(bool _type) { type = _type; }
    bool hasMessages() { return (bool)messages.size(); }
    Connection* getConnection() { return connection;}

    void attack(quint16 port, int amount);
    void help(quint16 port, int amount);
    void request(quint16 portHelper, quint16 portEnemy);

    void upgradeI();
    void upgradeD();
    void upgradeC();

    void deathRecountRealloc();

    int getI() { return In; }
    int getD() { return Dn; }
    int getC() { return Cn; }
    int getTemper() { return temper; }
    bool getDead() { return dead; }
    int getINextRequire();
    int getDNextRequire();
    int getCNextRequire();
    QString getMessage() { return messages.takeFirst(); }
};

#endif // CORE_H