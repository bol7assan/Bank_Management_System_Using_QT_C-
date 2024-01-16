#include "Server.h"

Server::Server(QObject *parent)
    : QTcpServer(parent), logger("Server")
{
    logger.log("Object Created.");
    if (!listen(QHostAddress::LocalHost, 54321)) {
        logger.log("Failed to start server: " + errorString());
    } else {
        logger.log("Listening on Port 54321");
    }
}

Server::~Server()
{
    //just to be sure
    for (auto it = clientThreads.begin(); it != clientThreads.end(); ++it)
    {
        it.value()->quit();
        it.value()->wait();
        delete it.value();
    }
    close();
    logger.log("All Threads have been closed");
    logger.log("Object Destroyed.");
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    QThread* clientThread = new QThread();
    clientThreads.insert(socketDescriptor, clientThread);

    ClientRunnable* clientRunnable = new ClientRunnable(socketDescriptor);
    clientRunnable->moveToThread(clientThread);

    connect(clientThread, &QThread::started, clientRunnable, &ClientRunnable::run);
    connect(clientRunnable, &ClientRunnable::clientDisconnected, this, &Server::handleClientDisconnected);
    connect(clientRunnable, &ClientRunnable::destroyed, clientThread, &QThread::quit);

    clientThread->start();
    logger.log(QString("Client connected with socket descriptor: %1").arg(socketDescriptor));
}

void Server::handleClientDisconnected(qintptr socketDescriptor)
{
    clientThreads.value(socketDescriptor)->quit();
    clientThreads.value(socketDescriptor)->wait();
    delete clientThreads.value(socketDescriptor);
    clientThreads.remove(socketDescriptor);
    logger.log(QString("Client disconnected with socket descriptor: %1").arg(socketDescriptor));
}
