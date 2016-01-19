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
    QHostAddress host; // айпи (локал в любом случае, по идее)
    int relationship; // -100 - злейший враг, 0 - нейтрал, 100 - лучший друг
    int useful; // 0 - связь бесполезна, 100 - очень полезна
    bool closed; // закрытый канал
    int prevRelate; // предрасположенность, -1 - плохая, 1 - хорошая
};

class Connection : public QObject
{
    Q_OBJECT

private:

    QList<connectTable> table;
    QList<QString> data;

    quint16 portRecieve;
    quint16 portMin;
    quint16 portMax;

    QUdpSocket* udpSocket;


private slots:
    void sendData(quint16 _port, int type); // запись на порт
    void readData(); // считывание с порта


public:
    Connection(int port);

    void send(quint16 port, int type);

    bool hasData() { return (bool)data.size(); }

    QString getData() { return data.takeLast(); }
    quint16 getPort() { return portRecieve; }
    connectTable getTable(int pos) { return table.at(pos); }
    int getTableSize() { return table.size(); }


};

#endif // CONNECTION_H
