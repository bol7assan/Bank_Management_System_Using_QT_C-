#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QFile>
#include <QMutex>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

#include "Logger.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(const QString &connectionName, QObject *parent = nullptr);
    ~DatabaseManager();
    void initializeDatabase();
    bool openConnection();
    void closeConnection();
    bool createTables();
    QJsonObject processRequest(QJsonObject requestJson);

private:
    QMutex mutex;
    QString connectionName;
    Logger logger;

    QJsonObject login(QJsonObject requestJson);
    QJsonObject getAccountNumber(QJsonObject requestJson);
    QJsonObject getAccountBalance(QJsonObject requestJson);
    QJsonObject createNewAccount(QJsonObject requestJson);
    QJsonObject deleteAccount(QJsonObject requestJson);
    QJsonObject fetchAllUserData(void);
    QJsonObject makeTransaction(QJsonObject requestJson);
    QJsonObject makeTransfer(QJsonObject requestJson);
    QJsonObject viewTransactionHistory(QJsonObject requestJson);
    QJsonObject updateUserData(QJsonObject requestJson);
};

#endif // DATABASEMANAGER_H
