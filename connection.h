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
    int relationship; // -5 - злейший враг, 0 - нейтрал, 5 - лучший друг
    int useful; // 0 - связь бесполезна, 10 - очень полезна
    int lostSignal; // "мертвая" связь
};

class Connection : public QObject
{
    Q_OBJECT

private:

    QList<connectTable> table; // таблица соединений
    QList<QString> data; // сохраненные отчеты о приемах

    quint16 portRecieve; // личный порт приема
    quint16 portMin; // ???
    quint16 portMax; // зачем

    int temper; // настроение

    QUdpSocket* udpSocket; // сокет


private slots:

    void readData(); // считывание с порта


public:
    Connection(int port, int _temper);

    void sendData(quint16 port, int type); // запись на порт
    void sendData(quint16 port, int type, int amount); // передача данных
    void deleteTable(int pos) { table.removeAt(pos); }
    void setLostSignal(int pos, int n) { table[pos].lostSignal = n; }
    void setRelationship(int pos, int n) { table[pos].relationship = n; }
    void setUseful(int pos, int n) { table[pos].useful = n; }
    void createTable(connectTable _table);

    bool hasData() { return (bool)data.size(); }

    void sortTable();

    QString getData() { return data.takeLast(); }
    quint16 getPort() { return portRecieve; }
    connectTable getTable(int pos) { return table.at(pos); }
    int getTableSize() { return table.size(); }


};

#endif // CONNECTION_H
