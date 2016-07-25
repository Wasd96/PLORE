#include "connection.h"

Connection::Connection(int port, int _temper, int _type, bool _silent) // создание модуля связи
{
    table.clear(); // очистка таблицы связей (на всякий случай)
    data.clear();

    temper = _temper;
    type = _type;
    silent = _silent;

    foundTable = new int[200];
    for (int i = 0; i < 200; i++) foundTable[i] = 0;

    udpSocket = new QUdpSocket(this); // создание сокета
    while (!udpSocket->bind(port, QUdpSocket::ShareAddress)) // инициализация
    {
       port++;
       if (port >= 50200) port = 50000;
    }
    portRecieve = udpSocket->localPort(); // сохранение личного порта


    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readData())); // прием данных
}

Connection::~Connection()
{
    delete udpSocket;
    delete foundTable;
}

void Connection::rebindPort(int port) // попытка получить нужный порт
{
    while (udpSocket->localPort() != port) { // потенциально бесконечный цикл(
        udpSocket->close();
        udpSocket->bind(port, QUdpSocket::DontShareAddress);
        portRecieve = port;
    }
}

void Connection::sendData(quint16 port, int Mtype) //подготовка и отправка данных
{
    QString outData;
    outData = QString::number(Mtype);

    if (Mtype == 0) // установка связи
    {
        outData = "0 "+QString::number(temper)+" "+QString::number(type);
    }
    /*if (Mtype == 1) // проверка связи
    if (Mtype == 2) // подтверждение связи
    if (Mtype == 6) // подтверждение боевой помощи
    if (Mtype == 80) // спавн бота (лаунчеру)
    if (Mtype == 88) // код самоубийства для процесса
    if (Mtype == 90) // Закончен уровень (посылка лаунчеру)
    */

    QByteArray datagram = outData.toUtf8();
    udpSocket->writeDatagram(datagram.data(),
                             datagram.size(),
                             QHostAddress::Broadcast, // важно - не везде localhost работает как надо
                             port);
}

void Connection::sendData(quint16 port, int Mtype, int amount)
{
    QString outData;
    outData = QString::number(Mtype)+" "+QString::number(amount);
    /*if (Mtype == 1) // сообщение о смерти
    if (Mtype == 3) // атака
    if (Mtype == 4) // помощь (передача памяти)
    */

    if (Mtype == 5) // просьба о помощи
    {
        // здесь amount = port врага
        int i;
        for (i = 0; i < table.size(); i++)
            if (table.at(i).port == amount) // сравниваем текущий порт с существующими
                break;
        outData = "5 " + QString::number(amount) + " " + QString::number(table.at(i).type);
    }

    QByteArray datagram = outData.toUtf8();
    udpSocket->writeDatagram(datagram.data(),
                             datagram.size(),
                             QHostAddress::Broadcast,
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

        if (silent && port != 45454)
            continue;

        QString str = datagram.data();
        QStringList strList = str.split(' ');

        QString rec = QString::number(port)+ " -> " + str; // создание сообщения
        data.append(rec); // сохранение сообщения

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

            if (strList.first() != "0") // системное сообщение
            {
                if (strList.first() == "88") // лаунчер сообщает, что пора умирать
                {
                    emit died(88);
                    break;
                }
                if (strList.first() == "80") // спавн червя
                {
                    emit died(80);
                    break;
                }
                if (strList.first() == "70") // включить связь
                {
                    if (strList.at(1) == QString::number(type))
                    {
                        setSilent(0);
                    }
                    break;
                }
                if (strList.first() == "71") // изменить видимость
                {
                    if (strList.at(1) == QString::number(type))
                    {
                        emit setVisible(1);
                    }
                    break;
                }
                if (strList.first() == "90") // конец уровня
                {
                    emit died(90);
                    break;
                }

                emit died(strList.at(1).toInt()); // 0 - прога, 1 - юзер, 2 - бот, 3 - троян
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
