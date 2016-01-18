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

public:
    Connection();

    void send(int type);



private slots:
    void sendData(int type); // запись на порт
    void readData(); // считывание с порта

private:

    QList<connectTable> table;

    quint16 portRecieve;
    quint16 portMin;
    quint16 portMax;

    QUdpSocket* udpSocketRead;
    QUdpSocket* udpSocketWrite;
};

#endif // CONNECTION_H
