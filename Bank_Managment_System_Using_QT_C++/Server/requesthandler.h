#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "databasemanager.h"
#include "logger.h"

class RequestHandler : public QObject
{
    Q_OBJECT

public:
    explicit RequestHandler(const QString &connectionName, QObject *parent = nullptr);
    ~RequestHandler();
    QByteArray handleRequest(QByteArray requestData);

signals:
    void responseReady(QByteArray responseData);

private:
    DatabaseManager *databaseManager;
    QString connectionName;
    Logger logger;

    QByteArray createResponse(QJsonObject responseJson);
};

#endif // REQUESTHANDLER_H
