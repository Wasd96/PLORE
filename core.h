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

    int Ii, Ci; // прирост памяти и активного ресурса

    bool dead; // мертв, пора удалять
    bool search; // флаг автопоиск
    int type; // тип проги 0 - норм 1 - юзер 2 - бот 3 - троян

    int timeToUpgrade; // желание улучшить параметры (от застоя)
    double coeff; // граница улучшения (просто коэффициент)

    int requestAttack; // порт, который просили атаковать (для юзера)
    int requestAttackSender; // порт просящего атаку

    QList<QString> messages;

public:
    Core();
    Core(int _I, int _D, int _C, int _temper, int _Ii, int _Ci, int _type, bool _silent);
    ~Core();

    void update();
    void updateUser();
    void updateWorm();
    void connectionSupport();
    void operateDataFromConnection();
    void coeffRecount();

    void send(quint16 port, int _type);
    void send(quint16 port, int _type, int amount);
    void setSearch(bool _search) { search = _search; }
    bool hasMessages() { return (bool)messages.size(); }
    Connection* getConnection() { return connection;}

    void attack(quint16 port, int amount);
    void help(quint16 port, int amount);
    void request(quint16 portHelper, quint16 portEnemy);

    void upgradeI();
    void upgradeD();
    void upgradeC();

    void deathRecountRealloc();
    void findConnections();


    void setD(int _D) { Dn = (double)_D; }

    int getI() { return In; }
    int getD() { return Dn; }
    int getC() { return Cn; }
    int getIi() { return Ii; }
    int getCi() { return Ci; }
    int getType() { return type; }
    bool getDead() { return dead; }
    int getINextRequire();
    int getDNextRequire();
    int getCNextRequire();
    QString getMessage() { return messages.takeFirst(); }
};

#endif // CORE_H
