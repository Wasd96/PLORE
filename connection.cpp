#include "connection.h"

Connection::Connection()
{
    table.clear();

    udpSocketRead = new QUdpSocket(this);
    udpSocketWrite = new QUdpSocket(this);

    udpSocketRead->bind(45454, QUdpSocket::ShareAddress);
    udpSocketWrite->bind(45455, QUdpSocket::ShareAddress);
    qDebug()<<udpSocketRead;
    QHostAddress host;
    host.setAddress(QHostAddress::Broadcast);
    qDebug()<<host;
    //0x1ebefb8
    //0x1eb - порт, efb8 - айпи

    connect(udpSocketRead, SIGNAL(readyRead()), this, SLOT(readData()));
}

void Connection::send(int type)
{
    sendData(type);
}

void Connection::sendData(int type)
{
    QString str = "test";
    QByteArray datagram = str.toUtf8();

    QHostAddress host;
    quint16 port = 45454;
    host.setAddress("255.255.255.255");
    QString sen = "Sending \""+str+"\" to host: " +
                         host.toString() + " to port: " + QString::number(port);
    qDebug() << sen;

    udpSocketWrite->writeDatagram(datagram.data(), datagram.size(),
                             host, port);
    qDebug() << "size: " << datagram.size();
}

void Connection::readData() {

    qDebug() << "\nNew message! ";

    QHostAddress host;
    quint16 port;
    while (udpSocketRead->hasPendingDatagrams()) {
        qDebug() << "recieving...";

        QByteArray datagram;
        datagram.resize(udpSocketRead->pendingDatagramSize());
        udpSocketRead->readDatagram(datagram.data(), datagram.size(), &host, &port);


        QString str = datagram.data();
        QString rec = str +
                             "    from IP:   " +
                             host.toString() +
                             "   port:   " +
                             QString::number(port)+
                             "\n---------";
        qDebug() << rec;
        qDebug() << datagram.data() << port << host;
        qDebug() << "size: " << datagram.size();
    }
    // otvet
    //QByteArray datagram_answer = "Delivered!";
    //udpSocketWrite->writeDatagram(datagram_answer.data(), datagram_answer.size(), host, port);
    //

}
