#include <QDateTime>
#include <QDebug>

#include "server.h"

Q_LOGGING_CATEGORY(SERVER, "SERVER")

Server::Server(QObject *parent) : QObject(parent),
    m_totalBytes(0), m_sendTimeNum(1)
{
    // Variables initialization and build connections
    m_server = new QTcpServer(this);
    m_sendSocket = new QTcpSocket(this);
    m_receiveSocket = new QTcpSocket(this);

    setCmdString();
    setErrorString();

    readSettings();
    connect(m_sendSocket, SIGNAL(readyRead()), this, SLOT(readReceipt()));
    connect(m_sendSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    connect(this,SIGNAL(error(QString)),this,SLOT(handleError(QString)));

    m_checkTimes = 0;

    m_loadSize = 3*1024;
    m_totalBytes = 0;
    m_bytesWritten = 0;
    m_bytesToWrite = 0;

    //connect(m_sendSocket, SIGNAL(connected()), this, SLOT(startTransferFile()));    // If connection is built, send connected signal, start session

}

Server::~Server()
{
}

// Connect to client and start session for sending plan and command
void Server::connectServer()
{
    QHostAddress ipAddress(m_sendIpAddress);    // Set the IP address of another computer
    m_sendSocket->connectToHost(ipAddress, m_sendPort);    // Connect

    //    qDebug() << "&&&&&&&&&&&&&&&&&&&&&&";
    //    qDebug() << "peerName:" << m_sendSocket->peerName();
    //    qDebug() << "peerAddress:" << m_sendSocket->peerAddress();
    //    qDebug() << "peerPort:" << m_sendSocket->peerPort();
    //    qDebug() << endl;
    //    qDebug() << "localAddress:" << m_sendSocket->localAddress();
    //    qDebug() << "localPort:" << m_sendSocket->localPort();
    //    qDebug() << "&&&&&&&&&&&&&&&&&&&&&&";
}

void Server::readSettings()
{
    QSettings *settings = new QSettings(SETTINGS_PATH, QSettings::IniFormat);
    m_receiveIpAddress = settings->value("Receive/IpAddress").toString();
    m_receivePort = settings->value("Receive/Port").toString().toUShort(0,10);
    m_sendIpAddress = settings->value("Send/IpAddress").toString();
    m_sendPort = settings->value("Send/Port").toString().toUShort(0,10);
    delete settings;
}

void Server::updateSettings()
{
    QSettings *settings = new QSettings(SETTINGS_PATH, QSettings::IniFormat);
    settings->setValue("Receive/IpAddress", m_receiveIpAddress);
    settings->setValue("Receive/Port", m_receivePort);
    settings->setValue("Send/IpAddress", m_sendIpAddress);
    settings->setValue("Send/Port", m_sendPort);
    delete settings;
}

void Server::setCmdString()
{
    //  TODO
    m_cmdList << "SEND COMMAND START"
              << "SEND COMMAND STOP"
              << "SEND COMMAND PAUSE"
              << "SEND COMMAND RESUME";
}

// Display error report
void Server::displayError(QAbstractSocket::SocketError)
{
    qCWarning(SERVER()) << SERVER().categoryName() << m_sendSocket->errorString();
    m_sendSocket->close();
    qCDebug(SERVER()) << SERVER().categoryName() << "Send socket was closed.";
}

void Server::setErrorString()
{
    m_errorList << "Successfully done."
                << "Failed to send enough bytes."
                << "Failed to check the receipt."
                << "Failed to receive enough bytes.";
}

void Server::handleError(QString errorString)
{
    qCWarning(SERVER()) << SERVER().categoryName() << errorString;
}

// Send treatment plan
void Server::sendPlan()
{
    if (m_sendSocket->state() == m_sendSocket->ConnectingState)
    {
        emit socketBlockError();
        qCWarning(SERVER()) << SERVER().categoryName() << "Socket block !";
    }
    else
    {
        connectServer();

        //  Check if the data has been all well written
        connect(m_sendSocket, SIGNAL(bytesWritten(qint64)),
                this, SLOT(writtenBytes(qint64)));

        qCDebug(SERVER()) << SERVER().categoryName() << "Sending plan...";

        encodePlan(&m_baOut);
        m_sendSocket->write(m_baOut);

        disconnect(m_sendSocket, SIGNAL(bytesWritten(qint64)),
                   this, SLOT(writtenBytes(qint64)));
    }
}

void Server::encodePlan(QByteArray *baBlock)
{
    QDataStream out(baBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);

    genReceipt(m_receipt);
    encodeSpot();

    out << qint64(0)
        << qint64(0)
        << m_hashX
        << m_hashY
        << m_hashZ
        << m_spotOrder
        << m_parameter.volt
        << m_parameter.totalTime
        << m_parameter.period
        << m_parameter.dutyCycle
        << m_parameter.coolingTime
        << m_receipt;

    qDebug() << "Spot order:" << m_spotOrder;
    qDebug() << "Volt:" << m_parameter.volt
             << "Total time:" << m_parameter.totalTime
             << "Period:" << m_parameter.period
             << "Duty cycle:" << m_parameter.dutyCycle
             << "Cooling time:" << m_parameter.coolingTime;
    qDebug() << "receipt:" << m_receipt;

    m_totalBytes = baBlock->size();
    qDebug() << "m_totalBytes:" << m_totalBytes;
    out.device()->seek(0);
    out << qint64(PLAN) << m_totalBytes;    // Find the head of array and write the haed information
}

//  load the list of 3D coordinates from the hash
void Server::encodeSpot()
{
    QHash<float, QList<Spot3DCoordinate> >::iterator i;
    for (i = m_spot3D.begin(); i != m_spot3D.end(); ++i)
    {
        float currentKey = i.key();
        QList<Spot3DCoordinate> currentList = i.value();
        QList<Coordinate> newListX, newListY, newListZ;
        int listSize = currentList.size();
        Spot3DCoordinate currentStruct;
        for (int j = 0; j < listSize; j++)
        {
            currentStruct = currentList.at(j);
            newListX.append(currentStruct.x);
            newListY.append(currentStruct.y);
            newListZ.append(currentStruct.z);
        }
        m_hashX[currentKey] = newListX;
        m_hashY[currentKey] = newListY;
        m_hashZ[currentKey] = newListZ;
    }
    qDebug() << "X:" << m_hashX;
    qDebug() << "Y:" << m_hashY;
    qDebug() << "Z:" << m_hashZ;
}

void Server::sendCommand(cmdType iType)
{
    if (m_sendSocket->state() == m_sendSocket->ConnectingState)
    {
        emit socketBlockError();
        qCWarning(SERVER()) << SERVER().categoryName() << "Socket block !";
    }
    else
    {
        connectServer();

        encodeCmd(&m_baOut,iType);

        qCDebug(SERVER()) << SERVER().categoryName() << "Start sending command ...";
        qDebug() << "Receipt:" << m_receipt;

        m_sendSocket->write(m_baOut);

        m_baOut.clear();

        //  TODO
        //  Add the receipt
        //    qDebug() << "Send finished";
        qCDebug(SERVER()) << SERVER().categoryName() << m_cmdList.at(iType - 1);
        //    qDebug() << "**************************************";
        emit sendingCompleted();
    }
}

void Server::encodeCmd(QByteArray *baBlock, cmdType iType)
{
    QDataStream out(baBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_6);

    genReceipt(m_receipt);

    out << qint64(COMMAND);
    out << qint64(iType)
        << m_receipt;
}

// Generate the log information of treatment plan sending
void Server::genReceipt(QString &receipt)
{
    QLocale curLocale(QLocale("C"));
    QLocale::setDefault(curLocale);
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = QLocale().toString(dateTime, "ddd,d MMM yyyy, hh:mm:ss");

    QString server = "ServerName";
    QString client = "ClientName";
    QString timeNum = "Time: " + QString::number(m_sendTimeNum, 10);
    receipt = "From: " + server + ", " + "To: " + client + ", " + timeNum + ", " + dateTimeString;
}

// Read receipt
void Server::readReceipt()
{    
    //  test the signal of readyRead
    //    qDebug() << "readyRead...";

    QDataStream in(m_sendSocket);
    in.setVersion(QDataStream::Qt_4_6);

    QString receipt;
    //  test
    //    qDebug() << "bytesAvail:" << m_sendSocket->bytesAvailable();
    in >> receipt;
    m_sendSocket->close();

    //  test
    //    qDebug() << "receipt:" << receipt;
    //  Check the consistency of the send-back data
    //  m_receivedInfo is updated outside the current thread
    //  not thread-safe
    if(m_receipt == receipt)
    {

        qDebug() << "Receipt checked.";
        //        qCDebug(SERVER()) << SERVER().categoryName() << ":" << "SUCESSFULLY SEND TREATMENT PLAN.";
        //        qDebug() << "**************************************";
        // Clear the variables
        m_receipt.clear();
        m_sendTimeNum += 1;
        m_baOut.clear();
        m_hashX.clear();
        m_hashY.clear();
        m_hashZ.clear();
        m_spot3D.clear();
        m_spotOrder.clear();
        emit sendingCompleted();
    }else
    {
        emit error(m_errorList[ErrorReadReceipt]);
    }
}

// Slot function to capture written bytes of socket
void Server::writtenBytes(qint64 bytesWrite)
{
    qDebug() << "Bytes Written:" << bytesWrite;

    m_writtenBytes = bytesWrite;

    if (m_writtenBytes != m_totalBytes)
    {
        emit error(m_errorList[ErrorSend]);
    }
    else
    {
        emit error(m_errorList[NoError]);
    }
}

QString Server::getLocalIP()
{
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address() &&
                (ipAddressesList.at(i).toString().indexOf("169") != (-1))) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    return ipAddress;
}

void Server::listen()
{
    if (m_receiveIpAddress.isEmpty())
    {
        m_receiveIpAddress = getLocalIP();
        updateSettings();
    }

    if (m_server->isListening() == false)
    {
        if (!m_server->isListening())
        {
            QHostAddress ipAddress(m_receiveIpAddress.toInt());
            if(!m_server->listen(ipAddress, m_receivePort))
            {
                qCWarning(SERVER()) << SERVER().categoryName() << m_server->errorString();
                m_server->close();
                return;
            }
        }
        qDebug() << "Listen IP:" << m_receiveIpAddress << "Port:" << m_receivePort << "OK";
        connect(m_server, SIGNAL(newConnection()) ,
                this, SLOT(acceptConnection()));    // Send newConnection() signal when a new connection is detected}
    }
}

void Server::acceptConnection()
{
    m_receiveSocket = m_server->nextPendingConnection();
    qDebug() << "m_receiveSocket:" << m_receiveSocket;
    connect(m_receiveSocket, SIGNAL(readyRead()), this, SLOT(receive()));
    connect(m_receiveSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));

    qDebug() << "Connection OK";
}

void Server::receive()
{
    QDataStream in(m_receiveSocket);
    in.setVersion(QDataStream::Qt_4_6);

    qCDebug(SERVER()) << SERVER().categoryName() << "Receiving data...";

    qint64 type;

    in >> type
            >> m_totalBytes
            >> m_status;

    qDebug() << "m_totalBytes:" << m_totalBytes;
    qDebug() << "m_status:" << m_status;

    m_receiveSocket->close();
    qCDebug(SERVER()) << SERVER().categoryName() << "RECEIVED PROGRESS UPDATE FINISHED.";
    emit receivingCompleted();
}


void Server::checkConnection()
{
    if (m_sendSocket->state() == m_sendSocket->ConnectingState)
    {
        emit socketBlockError();
        qCWarning(SERVER()) << SERVER().categoryName() << "Socket block !";
    }
    else
    {
        connectServer();

        connect(m_sendSocket, SIGNAL(bytesWritten(qint64)),
                this, SLOT(writtenBytes(qint64)));

        QDataStream out(&m_baOut, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_6);

        genReceipt(m_receipt);

        out << qint64(CHECK)
            << m_receipt;

        m_sendSocket->write(m_baOut);

        disconnect(m_sendSocket, SIGNAL(bytesWritten(qint64)),
                   this, SLOT(writtenBytes(qint64)));

        m_checkTimes += 1;
        qDebug() << "Check times: " << m_checkTimes;
    }
}


void Server::encodeFile()
{
    // Generate file name
    QLocale curLocale(QLocale("C"));
    QLocale::setDefault(curLocale);
    QDateTime dateTime = QDateTime::currentDateTime();
    QString dateTimeString = QLocale().toString(dateTime, "dMMMyyyy_hhmmss");

    m_fileName = "TCP_File_" + dateTimeString + ".HIFU";
    qDebug() << m_fileName;

    // QDir::setCurrent("D:/");
    m_writeFile.setFileName(m_fileName);
    if (!m_writeFile.open(QIODevice::WriteOnly))  // EMIT ERROR
        qDebug() << "Error";

    QDataStream outData(&m_writeFile);

    //    outData << (quint32)0xA0B0C0D0;
    //    outData << (qint32)460;
    outData.setVersion(QDataStream::Qt_4_6);

    encodeSpot();

    outData << m_hashX
            << m_hashY
            << m_hashZ
            << m_spotOrder
            << m_parameter.volt
            << m_parameter.totalTime
            << m_parameter.period
            << m_parameter.dutyCycle
            << m_parameter.coolingTime;

    qDebug() << "Spot order:" << m_spotOrder;
    qDebug() << "Volt:" << m_parameter.volt
             << "Total time:" << m_parameter.totalTime
             << "Period:" << m_parameter.period
             << "Duty cycle:" << m_parameter.dutyCycle
             << "Cooling time:" << m_parameter.coolingTime;

    qDebug() << "Encode file finished.";

    m_writeFile.close();
}


void Server::sendFile()
{
    if (m_sendSocket->state() == m_sendSocket->ConnectingState)
    {
        emit socketBlockError();
        qCWarning(SERVER()) << SERVER().categoryName() << "Socket block !";
    }
    else
    {
        encodeFile();
        m_bytesWritten = 0;
        connectServer();
        startTransferFile();
    }
}


void Server::startTransferFile()
{
    connect(m_sendSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(updateFileProgress(qint64)));

    m_readFile = new QFile(m_fileName);
    if(!m_readFile->open(QFile::ReadOnly))
    {
        qDebug() << "Open file failed !";
        return;
    }
    m_totalBytes = m_readFile->size();

    QDataStream outStream(&m_baOut, QIODevice::WriteOnly);
    outStream.setVersion(QDataStream::Qt_4_6);
    // QString currentFileName = m_fileName.right(m_fileName.size() - m_fileName.lastIndexOf('/')-1);
    outStream << qint64(0) << qint64(0) << qint64(0) << m_fileName;    // Write total bytes space, filename space, filename in order
    qDebug() << "currentFileName:" << m_fileName;

    m_totalBytes += m_baOut.size();    // m_totalBytes is the bytes information of filename space and the actual bytes of the file

    outStream.device()->seek(0);
    outStream << qint64(PLAN) << m_totalBytes << qint64((m_baOut.size() - sizeof(qint64)*2));     // Return the start of m_outBlock, replace two qint64 spaces by actual length information

    m_bytesToWrite = m_totalBytes - m_sendSocket->write(m_baOut);        // The rest data length after the head information

    qDebug() << "Write file head finished...";
    qDebug() << "BytesToWrite:" << m_bytesToWrite;

    m_baOut.resize(0);
}


void Server::updateFileProgress(qint64 numBytes)
{
    m_bytesWritten += (int)numBytes;    // Length of sent data

    if(m_bytesToWrite > 0)    // If any data has already been sent
    {
        m_baOut = m_readFile->read(qMin(m_bytesToWrite, m_loadSize));    // The data length of every send progress, here 4KB as default, if less than it, send the rest
        m_bytesToWrite -= (int)m_sendSocket->write(m_baOut);    // Length of remaining data

        qDebug() << "Block wirte finished...";
        qDebug() << "BytesWritten:" << m_bytesWritten;
        qDebug() << "BytesToWrite:" << m_bytesToWrite;

        m_baOut.resize(0);    // Clear the data buffer
    }
    else
    {
        m_readFile->close();    // If no data is sent, close file
    }

    if(m_bytesWritten == m_totalBytes)    // Send over
    {
        m_readFile->close();
        m_sendSocket->close();
        qDebug() << "All write finished.";
        disconnect(m_sendSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(updateFileProgress(qint64)));
        m_bytesToWrite = 0;
        m_bytesWritten = 0;
        m_totalBytes = 0;
    }
    else
    {
        qDebug() << "m_bytesWritten:" << m_bytesWritten;
        qDebug() << "m_totalBytes:" << m_totalBytes;
        qDebug() << "Update error !";
    }
}
