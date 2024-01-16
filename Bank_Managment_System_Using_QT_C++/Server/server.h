#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QThread>
#include <QMap>
#include "ClientRunnable.h"
#include "Logger.h"

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server(QObject *parent = nullptr);
    ~Server();

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void handleClientDisconnected(qintptr socketDescriptor);

private:
    QMap<qintptr, QThread*> clientThreads;
    Logger logger;
};

#endif // SERVER_H
