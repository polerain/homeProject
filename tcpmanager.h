#ifndef TCPMANAGER_H
#define TCPMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QDebug>
#include <QTimer>

class TcpManager : public QObject
{
    Q_OBJECT
public:
    static TcpManager &instance();

    // Connect to a simulated device server
    void connectToDevice(const QString &ip, quint16 port);
    void disconnectFromDevice();

    // Send command to device (e.g., "LIGHT_ON", "AC_26")
    void sendCommand(const QString &cmd);

    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &msg);
    void dataReceived(const QString &data); // Received status/data from device

private slots:
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onConnected();
    void onDisconnected();

private:
    explicit TcpManager(QObject *parent = nullptr);
    ~TcpManager();
    TcpManager(const TcpManager &) = delete;
    TcpManager &operator=(const TcpManager &) = delete;

    QTcpSocket *m_socket;
    QString m_currentIp;
    quint16 m_currentPort;
    bool m_isConnected;
};

#endif // TCPMANAGER_H
