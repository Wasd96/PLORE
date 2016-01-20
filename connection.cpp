#include "connection.h"

Connection::Connection(int port)
{
    table.clear();

    portRecieve = port;
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(portRecieve, QUdpSocket::ShareAddress);
    qDebug()<<udpSocket;

    QHostAddress host;
    host.setAddress(QHostAddress::Broadcast);
    qDebug()<<host;
    //0x1ebefb8
    //0x1eb - порт, efb8 - айпи

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readData()));
}

void Connection::send(quint16 port, int type)
{
    sendData(port, type);
}

void Connection::sendData(quint16 port, int type)
{
    QString str = "test";
    QByteArray datagram = str.toUtf8();

    QHostAddress host;
    host.setAddress("255.255.255.255");
    /*QString sen = "Sending \""+str+"\" to host: " +
                   host.toString() + " to port: " +
                   QString::number(port);
    qDebug() << sen;*/

    udpSocket->writeDatagram(datagram.data(), datagram.size(),
                             host, port);
    qDebug() << "size: " << datagram.size();
}

void Connection::readData() {

    qDebug() << "\nNew message! ";

    QHostAddress host;
    quint16 port;

    while (udpSocket->hasPendingDatagrams()) {
        qDebug() << "recieving...";

        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(), &host, &port); // прием


        QString str = datagram.data();
        QString rec = QString::number(port)+ " -> " + str;

        data.append(rec); // сохранение сообщения
        qDebug() << rec;
        qDebug() << datagram.data() << port << host;
        qDebug() << "size: " << datagram.size();

        // проверка наличия такого соединения
        bool exist = false;
        for (int i = 0; i < table.size(); i++)
        {
            if (table.at(i).port == port)
            {
                exist = true;
                break;
            }
        }

        if (exist)
        {
            // если было
            str = "this " + QString::number(port) + " connection already exist!";
            qDebug() << str;
            data.append(str);
        }
        else // это новое соединение
        {
            connectTable newTable;
            newTable.host = host;
            newTable.port = port;
            newTable.closed = false;
            newTable.relationship = 0;
            newTable.useful = 0;
            newTable.prevRelate = 0;

            table.append(newTable);

            sendData(port, 0); // ответное соединение

            str = "New connection created: " + host.toString() + " "+ QString::number(port);
            qDebug() << str;
            data.append(str);
        }
    }

}
