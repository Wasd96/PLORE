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

    double In, Cn; // кол-во параметров каждого типа
    int Dn;

    double Ii, Ci; // прирост памяти и активного ресурса

    bool dead; // мертв, пора удалять
    bool search; // флаг автопоиск
    int searchTimeOut;
    int type; // тип проги 0 - норм 1 - юзер 2 - бот 3 - сервер

    int timeToUpgrade; // желание улучшить параметры (от застоя)
    double coeff; // граница улучшения (просто коэффициент)

    int requestAttack; // порт, который просили атаковать (для юзера)
    int requestAttackSender; // порт просящего атаку

    double INextRequired, DNextRequired, CNextRequired;

    QList<QString> messages; // лог сообщений

    int requestHelpTimeOut; // время задержки для запроса помощи (от спама)

public:
    bool modules[9]; // "модули", функционал

public:
    Core();
    Core(double _I, int _D, double _C, int _temper, double _Ii, double _Ci, int _type, bool _silent);
    ~Core();

    void update();
    void updateUser();
    void updateWorm();
    void connectionSupport();
    void operateDataFromConnection();
    void coeffRecount();
    void nextRecount();

    void send(quint16 port, QString str);
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


    void setD(int _D) { Dn = _D; }

    int getI() { return (int)In; }
    int getD() { return (int)Dn; }
    int getC() { return (int)Cn; }
    double getIi() { return Ii; }
    double getCi() { return Ci; }
    int getType() { return type; }
    bool getDead() { return dead; }
    int getINextRequire() { return INextRequired; }
    int getDNextRequire() { return DNextRequired; }
    int getCNextRequire() { return CNextRequired; }
    QString getMessage() { return messages.takeFirst(); }
};

#endif // CORE_H
