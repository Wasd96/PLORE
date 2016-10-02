#include "connection.h"

Connection::Connection(int port, int _temper, int _type, bool _silent) // создание модуля связи
{
    table.clear(); // очистка таблицы связей (на всякий случай)
    data.clear();

    temper = _temper;
    type = _type;
    silent = _silent;

    foundTable = new int[200];
    for (int i = 10; i < 200; i++) foundTable[i] = 0;

    udpSocket = new QUdpSocket(this); // создание сокета
    if (port < 50010 && port > 50000)
        port = 50010+rand()%190;
    while (!udpSocket->bind(port, QUdpSocket::ShareAddress)) // инициализация
    {
       port++;
       if (port >= 50200) port = 50010;
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

void Connection::sendData(quint16 port, QString str)
{
    QByteArray datagram = str.toUtf8();
    udpSocket->writeDatagram(datagram.data(),
                             datagram.size(),
                             QHostAddress::Broadcast,
                             port);
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

        int numb = -1; // проверка наличия такого соединения
        for (int i = 0; i < table.size(); i++)
        {
            if (table.at(i).port == port) // сравниваем текущий порт с существующими
            {
                numb = i;
                break;
            }
        }


        if (numb != -1) // если было
        {
            if (strList.first() == "1") // проверка связи
            {
                sendData(port, 2);
            }
            if (strList.first() == "2") // подтверждение связи
            {
                table[numb].lostSignal = 0; // обновление тайм-аута
            }

            if (strList.first() == "3") // атака
            {
                if (table.at(numb).useful == 0) // если пользы нет
                {
                    table[numb].relationship-=6; // снижение отношений
                    if (table.at(numb).relationship < -50)
                        table[numb].relationship = -50;
                }
                else // сначала снижается польза
                {
                    table[numb].useful-=20;
                    if (table.at(numb).useful < 0)
                    {
                        table[numb].relationship+=table[numb].useful/2;
                        if (table.at(numb).relationship < -50)
                            table[numb].relationship = -50;
                        table[numb].useful = 0;
                    }
                }
            }
            // помощь памятью обрабатывается в core, так как нужно кол-во "памяти"

            if (strList.first() == "6") // ответ на запрос помощи в бою (успешная атака)
            {
                table[numb].useful += 15; // добро увеличивается ^^
                if (table.at(numb).useful > 50)  table[numb].useful = 50;
                table[numb].relationship+=7; // улучшение отношений
                if (table.at(numb).relationship > 50) table[numb].relationship = 50;
            }

        }
        else // такого соединения нет
        {
            if (strList.first() == "0") // установка связи
            {
                connectTable newTable;
                newTable.relationship = (strList.at(1).toInt() + temper)/2; // отношение
                newTable.type = (strList.at(2).toInt()); // тип проги
                if (newTable.relationship > 0)
                    newTable.useful = rand()%(newTable.relationship+1);
                else
                    newTable.useful = rand()%2;
                newTable.port = port;
                newTable.lostSignal = 0;
                newTable.selected = false;
                if (table.size() == 0)
                    newTable.selected = true;

                createTable(newTable); // добавление в таблицу
                sendData(port, 0); // ответное соединение
                                   //(чтобы отправитель знал о существовании принявшего)

                str = "Новое соединение: " + QString::number(port%1000); // из-за этой строчки были багованные
                data.append(str);                                        // порты с номерами команд
            }

            if (strList.first() != "0") // системное сообщение
            {
                if (strList.first() == "88") // лаунчер сообщает, что пора умирать
                {
                    emit died(88);
                    break;
                }
                if (strList.first() == "80") // спавн червя / сервер идет искать
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

                if (portRecieve == 45454) // это лаунчер
                {
                    emit died(strList.at(1).toInt()); // 0 - прога, 1 - юзер, 2 - бот, 3 - Сервер
                    break;
                }
                data.append(str);
            }
        }
    }
}


void Connection::createTable(connectTable _table)
{
    table.append(_table);
    sortTable();
}

void Connection::deleteTable(int pos)
{
    if (table.at(pos).selected == true)
        setSelectedConnection(-1);
    ignoreConnectionChange = true;
    table.removeAt(pos);
    ignoreConnectionChange = false;
}


void Connection::setSelectedConnection(int index)
{
    for (int i = 0; i < table.size(); i++)
        table[i].selected = false;

    if (index >= 0 && index < table.size() && !ignoreConnectionChange)
    {
        table[index].selected = true;

    }
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
                //if (table.at(i).selected) setSelectedConnection(i+1);
                //else if (table.at(i+1).selected) setSelectedConnection(i); //так же сдвигаем и выбранную строку
                table.swap(i, i+1);
                change = true;
            }
            else
            {
                if (table.at(i).relationship == table.at(i+1).relationship) // если отношение одинаковое
                {
                    if (table.at(i).useful > table.at(i+1).useful) // то сортировка по пользе
                    {
                        //if (table.at(i).selected) setSelectedConnection(i+1);
                        //else if (table.at(i+1).selected) setSelectedConnection(i);
                        table.swap(i, i+1);
                        change = true;
                    } // аттракцион - скатись по горке!
                }
            }
        }
    }
}
