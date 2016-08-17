#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QtNetwork/QtNetwork>
#include <QList>
#include <QDebug>

class QUdpSocket;

struct connectTable
{
    quint16 port; // порт
    int relationship; // -50 - злейший враг, 0 - нейтрал, 50 - лучший друг
    int useful; // 0 - связь бесполезна, 50 - очень полезна
    int lostSignal; // "мертвая" связь
    int type; // -1 - лаунчер, 0 - норм, 1 - юзер, 2 - юзер, 3 - Сервер
    bool silent; // режим радиомолчания
    bool selected; // выбрано в списке
};

class Connection : public QObject
{
    Q_OBJECT

private:

    QList<connectTable> table; // таблица соединений
    QList<QString> data; // сохраненные отчеты о приемах

    quint16 portRecieve; // личный порт приема

    int selectedConnection; // выбранная строка в таблице соединений

    int* foundTable; // массив поиска портов для ускорения

    int temper;  // настроение, характер, -50 - злой, +50 - добрый
    int type; // 0 - норм, 1 - юзер, 2 - бот, 3 - Сервер
    bool silent;

    QUdpSocket* udpSocket; // сокет

public:
    bool ignoreConnectionChange;

private slots:

    void readData(); // считывание с порта


public:
    Connection(int port, int _temper, int _type, bool _silent);
    ~Connection();
    void rebindPort(int port);

    void sendData(quint16 port, QString str);
    void sendData(quint16 port, int Mtype); // запись на порт
    void sendData(quint16 port, int Mtype, int amount); // передача данных
    void deleteTable(int pos);
    void setLostSignal(int pos, int n) { table[pos].lostSignal = n; }
    void setRelationship(int pos, int n) { table[pos].relationship = n; }
    void setUseful(int pos, int n) { n>0?table[pos].useful=n:table[pos].useful=0; }
    void setTemper(int _temper) { temper = _temper; }
    void createTable(connectTable _table);
    void setSelectedConnection(int index);
    void setSilent(bool _silent) { silent = _silent; }
    void setFoundTableAtInc(int index) { (foundTable[index])++; }

    bool hasData() { return (bool)data.size(); }
    void read() { readData(); } // оболочка

    void sortTable();

    QUdpSocket* getUdpSocket() { return udpSocket; }
    QString getData() { return data.takeFirst(); }
    quint16 getPort() { return portRecieve; }
    connectTable getTable(int pos) { return table.at(pos); }
    int getTableSize() { return table.size(); }
    int getFoundTableAt(int index) { return foundTable[index]; }
    int getSelectedConnection() { return selectedConnection; }
    int getType() { return type; }
    int getTemper() { return temper; }
    bool getSilent() { return silent; }

signals:
    void died(int type);
    void setVisible(bool visible);

};

#endif // CONNECTION_H
