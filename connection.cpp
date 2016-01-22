#include "connection.h"

Connection::Connection(int port, int _temper) // создание модуля связи
{
    table.clear(); // очистка таблицы связей (на всякий случай)
    data.clear();

    temper = _temper;

    udpSocket = new QUdpSocket(this); // создание сокета
    while (!udpSocket->bind(port, QUdpSocket::ShareAddress)) //инициализация
    {
       port++;
       if (port >= 50200) port = 50000;
    }
    portRecieve = udpSocket->localPort(); // сохранение личного порта


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
    if (type == 6) // подтверждение боевой помощи
    {
        outData = "6 ";
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
        outData = "3 " + QString::number(amount);
    }
    if (type == 4) // помощь (передача памяти)
    {
        outData = "4 " + QString::number(amount);
    }
    if (type == 5) // просьба о помощи
    {
        outData = "5 " + QString::number(amount); // здесь amount = port врага
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
                strList.first() != "4" &&
                strList.first() != "5" &&
                strList.first() != "6")
        {
            QString rec = QString::number(port)+ " -> " + str; // создание отчета
            data.append(rec); // сохранение отчета
        }
        if (strList.first() == "3" ||
                strList.first() == "4"||
                strList.first() == "5") // передача данных о сложных сообщениях
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
                        if (table.at(i).useful == 0) // если пользы нет
                        {
                            table[i].relationship--; // снижение отношений
                            if (table.at(i).relationship < -5)
                                table[i].relationship = -5;
                        }
                        else // сначала снижается польза
                        {
                            table[i].useful--;
                            if (table.at(i).useful < 0)
                                table[i].useful = 0;
                        }


                        break;
                    }
                }
            }

            if (strList.first() == "4" || strList.first() == "6") // помощь (памятью или боевая)
            {
                for (int i = 0; i < table.size(); i++)
                {
                    if (table.at(i).port == port) // выбор нужной записи
                    {
                        table[i].useful += 2; // добро увеличивается ^^
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
                sortTable();
                sendData(port, 0); // ответное соединение
                                   //(чтобы отправитель знал о существовании принявшего)
            }


            str = "Новое соединение: " + QString::number(port);
            data.append(str);
        }
    }

}


void Connection::createTable(connectTable _table)
{
    table.append(_table);
    sortTable();
}

void Connection::sortTable() // сортировка по возрастанию
{
    bool change = true;
    while (change)
    {
        change = false;
        for (int i = 0; i < table.size()-1; i++)
        {
            if (table.at(i).relationship > table.at(i+1).relationship)
            {
                table.swap(i, i+1);
                change = true;
            }
            else
            {
                if (table.at(i).relationship == table.at(i+1).relationship)
                {
                    if (table.at(i).useful > table.at(i+1).useful)
                    {
                        table.swap(i, i+1);
                        change = true;
                    }
                }
            }
        }
    }
}
