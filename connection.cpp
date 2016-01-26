#include "connection.h"

Connection::Connection(int port, int _temper, int _type) // создание модуля связи
{
    table.clear(); // очистка таблицы связей (на всякий случай)
    data.clear();

    temper = _temper;
    type = _type;

    udpSocket = new QUdpSocket(this); // создание сокета
    while (!udpSocket->bind(port, QUdpSocket::ShareAddress)) //инициализация
    {
       port++;
       if (port >= 50200) port = 50000;
       // при alpha 0.3.2. было замечено странное поведение
       // на другом компе. Возможно, с этими портами что то не так
       // заняты, например, хотя ShareAddress...
    }
    portRecieve = udpSocket->localPort(); // сохранение личного порта

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readData())); // прием данных
}

void Connection::sendData(quint16 port, int Mtype) //подготовка и отправка данных
{
    QString outData;
    if (Mtype == 0) // установка связи
    {
        outData = "0 "+QString::number(temper)+" "+QString::number(type);
    }
    if (Mtype == 1) // проверка связи
    {
        outData = "1 ";
    }
    if (Mtype == 2) // подтверждение связи
    {
        outData = "2 ";
    }
    if (Mtype == 6) // подтверждение боевой помощи
    {
        outData = "6 ";
    }
    if (Mtype == 88) // код самоубийства для процесса
    {
        outData = "88 ";
    }


    QByteArray datagram = outData.toUtf8();
    udpSocket->writeDatagram(datagram.data(),
                             datagram.size(),
                             QHostAddress::LocalHost,
                             port);
}

void Connection::sendData(quint16 port, int Mtype, int amount)
{
    QString outData;
    if (Mtype == 3) // атака
    {
        outData = "3 " + QString::number(amount);
    }
    if (Mtype == 4) // помощь (передача памяти)
    {
        outData = "4 " + QString::number(amount);
    }
    if (Mtype == 5) // просьба о помощи
    {
        // здесь amount = port врага
        int index;
        for (int i = 0; i < table.size(); i++)
        {
            if (table.at(i).port == amount) // сравниваем текущий порт с существующими
            {
                index = i;
                break;
            }
        }
        outData = "5 " + QString::number(amount) + " " + QString::number(table.at(index).type);
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
                strList.first() != "6" &&
                strList.first() != "88")
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


        bool exist = false; // проверка наличия такого соединения
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
        else // такого соединения нет
        {
            if (strList.first() == "0") // установка связи
            {
                connectTable newTable;
                newTable.relationship = (strList.at(1).toInt() + temper)/2; // отношение
                newTable.type = (strList.at(2).toInt()); // тип проги
                newTable.useful = rand()%2;
                newTable.port = port;
                newTable.lostSignal = 0;

                table.append(newTable); // добавление в таблицу
                sortTable();
                sendData(port, 0); // ответное соединение
                                   //(чтобы отправитель знал о существовании принявшего)

                str = "Новое соединение: " + QString::number(port%1000);
                data.append(str);
            }

            if (strList.first() == "1") // сообщение лаунчеру о смерти юзера
            {
                emit died(1);
            }

            if (strList.first() == "2") // сообщение лаунчеру о смерти бота
            {
                emit died(2);
            }

            if (strList.first() == "88") // лаунчер сообщает, что пора умирать
            {
                data.append(strList.first());
                emit died(0);
            }

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
                if (selectedConnection == i) selectedConnection = i+1;
                else if (selectedConnection == i+1) selectedConnection = i; //так же сдвигаем и выбранную строку
                table.swap(i, i+1);
                change = true;
            }
            else
            {
                if (table.at(i).relationship == table.at(i+1).relationship) // если отношение одинаковое
                {
                    if (table.at(i).useful > table.at(i+1).useful) // то сортировка по пользе
                    {
                        if (selectedConnection == i) selectedConnection = i+1;
                        else if (selectedConnection == i+1) selectedConnection = i;
                        table.swap(i, i+1);
                        change = true;
                    } // аттракцион - скатись по горке!
                }
            }
        }
    }
}
