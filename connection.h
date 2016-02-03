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
    int type; // -1 - лаунчер, 0 - бот, 1 - юзер
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

    int selectedConnection; // выбранная строка в таблице соединений

    int temper; // настроение
    int type; // 0 - бот, 1 - юзер

    QUdpSocket* udpSocket; // сокет


private slots:

    void readData(); // считывание с порта


public:
    Connection(int port, int _temper, int _type);
    ~Connection();

    void sendData(quint16 port, int Mtype); // запись на порт
    void sendData(quint16 port, int Mtype, int amount); // передача данных
    void deleteTable(int pos) { table.removeAt(pos); }
    void setLostSignal(int pos, int n) { table[pos].lostSignal = n; }
    void setRelationship(int pos, int n) { table[pos].relationship = n; }
    void setUseful(int pos, int n) { table[pos].useful = n; }
    void createTable(connectTable _table);
    void setSelectedConnection(int index) { selectedConnection = index; }

    bool hasData() { return (bool)data.size(); }

    void sortTable();

    QString getData() { return data.takeFirst(); }
    quint16 getPort() { return portRecieve; }
    connectTable getTable(int pos) { return table.at(pos); }
    int getTableSize() { return table.size(); }
    int getSelectedConnection() { return selectedConnection; }
    int getType() { return type; }

signals:
    void died(int type);

};

#endif // CONNECTION_H
