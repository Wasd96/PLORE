#include "connection.h"

Connection::Connection(int port, int _temper) // создание модуля связи
{
    table.clear(); // очистка таблицы связей (на всякий случай)

    temper = _temper;
    portRecieve = port; // сохранение личного порта
    udpSocket = new QUdpSocket(this); // создание сокета
    udpSocket->bind(portRecieve, QUdpSocket::ShareAddress); //инициализация

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readData())); // прием данных
}

void Connection::sendData(quint16 port, int type) //подготовка и отправка данных
{
    QString outData;
    if (type == 0) // установка связи
    {
        outData = "0 "+QString::number(temper);
    }
    if (type == 1) // проверка связи
    {
        outData = "1";
    }
    if (type == 2) // подтверждение связи
    {
        outData = "2";
    }


    QByteArray datagram = outData.toUtf8();
    udpSocket->writeDatagram(datagram.data(),
                             datagram.size(),
                             QHostAddress::LocalHost,
                             port);
}

void Connection::sendData(quint16 port, int type, int amount)
{
    QString outData;
    if (type == 3) // атака
    {
        outData = "3 "+QString::number(amount);
    }
    if (type == 4) // помощь
    {
        outData = "4 "+QString::number(amount);
    }


    QByteArray datagram = outData.toUtf8();
    udpSocket->writeDatagram(datagram.data(),
                             datagram.size(),
                             QHostAddress::LocalHost,
                             port);
}


void Connection::readData() // прием данных
{
    QHostAddress host;
    quint16 port;

    while (udpSocket->hasPendingDatagrams()) {

        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(),
                                datagram.size(),
                                &host,
                                &port); // прием

        QString str = datagram.data();
        QStringList strList = str.split(' ');

        if (strList.first() != "0" &&
                strList.first() != "1" &&
                strList.first() != "2" &&
                strList.first() != "3" &&
                strList.first() != "4")
        {
            QString rec = QString::number(port)+ " -> " + str; // создание отчета
            data.append(rec); // сохранение отчета
        }
        if (strList.first() == "3" || strList.first() == "4") // передача данных об атаке или помощи в виде отчета
        {
            data.append(QString::number(port) + " " + str);
        }

        // проверка наличия такого соединения
        bool exist = false;
        for (int i = 0; i < table.size(); i++)
        {
            if (table.at(i).port == port) // сравниваем текущий порт с существующими
            {
                exist = true;
                break;
            }
        }

        if (exist) // если было
        {
            if (strList.first() == "1") // проверка связи
            {
                sendData(port, 2);
            }
            if (strList.first() == "2") // подтверждение связи
            {
                for (int i = 0; i < table.size(); i++)
                {
                    if (table.at(i).port == port) // выбор нужной записи
                    {
                        table[i].lostSignal = 0; // обновление тайм-аута
                        break;
                    }
                }
            }


            if (strList.first() == "3") // атака
            {
                for (int i = 0; i < table.size(); i++)
                {
                    if (table.at(i).port == port) // выбор нужной записи
                    {
                        table[i].useful--;
                        if (table.at(i).useful < 0)  table[i].useful = 0;
                        table[i].relationship--; // снижение отношений
                        if (table.at(i).relationship < -5) table[i].relationship = -5;
                        break;
                    }
                }
            }

            if (strList.first() == "4") // помощь памятью
            {
                for (int i = 0; i < table.size(); i++)
                {
                    if (table.at(i).port == port) // выбор нужной записи
                    {
                        table[i].useful++;
                        if (table.at(i).useful > 10)  table[i].useful = 10;
                        table[i].relationship++; // улучшение отношений
                        if (table.at(i).relationship > 5) table[i].relationship = 5;
                        break;
                    }
                }
            }


        }
        else // это новое соединение
        {
            connectTable newTable;

            if (strList.first() == "0") // установка связи
            {
                newTable.relationship = (strList.at(1).toInt() + temper)/2; // отношение
                newTable.useful = rand()%2;
                newTable.port = port;
                newTable.lostSignal = 0;

                table.append(newTable); // добавление в таблицу
                sendData(port, 0); // ответное соединение
                                   //(чтобы отправитель знал о существовании принявшего)
            }



            str = "Новое соединение: " + QString::number(port);
            data.append(str);
        }
    }

}
