#include "tcpmanager.h"

TcpManager::TcpManager(QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this)), m_isConnected(false)
{
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpManager::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, &TcpManager::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpManager::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpManager::onErrorOccurred);
}

TcpManager::~TcpManager()
{
    if (m_socket->isOpen())
    {
        m_socket->close();
    }
}

TcpManager &TcpManager::instance()
{
    static TcpManager instance;
    return instance;
}

void TcpManager::connectToDevice(const QString &ip, quint16 port)
{
    // Check if already connected or connecting
    if (m_socket->state() == QAbstractSocket::ConnectedState)
    {
        if (m_currentIp == ip && m_currentPort == port)
        {
            qDebug() << "Already connected to this device.";
            return;
        }
        disconnectFromDevice();
    }
    else if (m_socket->state() != QAbstractSocket::UnconnectedState)
    {
        m_socket->abort(); // Abort current connection attempt
    }

    m_currentIp = ip;
    m_currentPort = port;

    qDebug() << "Connecting to" << ip << ":" << port;
    m_socket->connectToHost(QHostAddress(ip), port);
}

void TcpManager::disconnectFromDevice()
{
    if (m_socket->isOpen())
    {
        m_socket->disconnectFromHost();
        // Wait briefly or just let the signal handle it
    }
}

void TcpManager::sendCommand(const QString &cmd)
{
    if (!m_isConnected)
    {
        qDebug() << "Cannot send command: Not connected";
        emit errorOccurred("设备未连接");
        return;
    }

    QByteArray data = cmd.toUtf8();
    m_socket->write(data);
    m_socket->flush();
    qDebug() << "Sent command:" << cmd;
}

bool TcpManager::isConnected() const
{
    return m_isConnected;
}

void TcpManager::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    QString strData = QString::fromUtf8(data);
    qDebug() << "Received data:" << strData;
    emit dataReceived(strData);
}

void TcpManager::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorMsg = m_socket->errorString();
    qDebug() << "Socket Error:" << errorMsg;
    emit errorOccurred(errorMsg);
}

void TcpManager::onConnected()
{
    qDebug() << "Connected to device!";
    m_isConnected = true;
    emit connected();
}

void TcpManager::onDisconnected()
{
    qDebug() << "Disconnected from device.";
    m_isConnected = false;
    emit disconnected();
}
