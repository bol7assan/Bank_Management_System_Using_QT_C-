#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(const QString &connectionName, QObject *parent)
    : QObject(parent), connectionName(connectionName), logger("DatabaseManager")
{
    logger.log("DatabaseManager Object Created.");
    QSqlDatabase dbConnection = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    dbConnection.setDatabaseName("bankdatabase.db");
}

DatabaseManager::~DatabaseManager()
{
    QSqlDatabase::removeDatabase(connectionName);
    logger.log("DatabaseManager Object Destroyed.");
}

bool DatabaseManager::openConnection()
{
    QSqlDatabase dbConnection = QSqlDatabase::database(connectionName);
    if (!dbConnection.open())
    {
        logger.log(QString("Failed to Open database connection '%1'").arg(connectionName));
        return false;
    }

    logger.log(QString("Opened database connection '%1'").arg(connectionName));
    return true;
}

void DatabaseManager::closeConnection()
{
    QSqlDatabase dbConnection = QSqlDatabase::database(connectionName);

    if (dbConnection.isOpen())
    {
        dbConnection.close();
        logger.log(QString("Closed database connection '%1'").arg(connectionName));
    }
    else
    {
        logger.log(QString("Database connection '%1' is not open.").arg(connectionName));
    }
}


void DatabaseManager::initializeDatabase()
{
    QFile databaseFile("bankdatabase.db");
    if (databaseFile.exists())
    {
        logger.log("bankdatabase already exists.");
    }
    else
    {
        // Create the file if it doesn't exist
        if (databaseFile.open(QIODevice::WriteOnly))
        {
            databaseFile.close();
            logger.log("Created database file: bankdatabase.db");
            openConnection();
            createTables();
            closeConnection();
        }
        else
        {
            logger.log("Failed to create database file!");
        }
    }
}

QJsonObject DatabaseManager::processRequest(QJsonObject requestJson)
{
    QMutexLocker locker(&mutex);
    // Extract the request ID from the request JSON
    int requestId = requestJson["requestId"].toInt();

    QJsonObject responseJson;

    // Process the request based on the request ID
    switch(requestId)
    {
    case 0:
        responseJson = login(requestJson);
        break;
    case 1:
        responseJson = getAccountNumber(requestJson);
        break;
    case 2:
        responseJson = getAccountBalance(requestJson);
        break;
    case 3:
        responseJson = createNewAccount(requestJson);
        break;
    case 4:
        responseJson = deleteAccount(requestJson);
        break;
    case 5:
        responseJson = fetchAllUserData();
        break;
    case 6:
        responseJson = makeTransaction(requestJson);
        break;
    case 7:
        responseJson = makeTransfer(requestJson);
        break;
    case 8:
        responseJson = viewTransactionHistory(requestJson);
        break;
    case 9:
        responseJson = updateUserData(requestJson);
        break;
    case 10:
        responseJson = getAccountBalance(requestJson);
        break;
    case 11:
        responseJson = viewTransactionHistory(requestJson);
        break;
    default:
        // Handle unknown request
        logger.log("Unknown request");
        break;
    }

    // Add the response ID to the response JSON
    responseJson["responseId"] = requestId;

    return responseJson;
}

bool DatabaseManager::createTables()
{
    QSqlDatabase dbConnection = QSqlDatabase::database(connectionName);
    QSqlQuery query(dbConnection);

    // Begin transaction
    if (!dbConnection.transaction())
    {
        logger.log("Failed to start a transaction for table creation.");
        query.finish();
        return false;
    }

    // Create Accounts table
    const QString prep_accounts =
        "CREATE TABLE Accounts (AccountNumber INTEGER PRIMARY KEY AUTOINCREMENT,"
        " Username TEXT COLLATE NOCASE UNIQUE NOT NULL, Password TEXT NOT NULL,"
        " Admin BOOLEAN);";
    if (!query.exec(prep_accounts))
    {
        logger.log("Failed execution for Accounts table.");
        logger.log("Error: " + query.lastError().text());
        dbConnection.rollback();
        query.finish();
        return false;
    }

    // Insert default admin account
    const QString insert_default_admin =
        "INSERT INTO Accounts (Username, Password, Admin) "
        "VALUES ('admin', 'admin', 1);";
    if (!query.exec(insert_default_admin))
    {
        logger.log("Failed to insert default admin account.");
        logger.log("Error: " + query.lastError().text());
        dbConnection.rollback();
        query.finish();
        return false;
    }

    // Create Users_Personal_Data table
    const QString prep_users_personal_data =
        "CREATE TABLE Users_Personal_Data (AccountNumber INTEGER PRIMARY KEY, Name TEXT,"
        " Age INTEGER CHECK(Age >= 18 AND Age <= 120), Balance REAL, FOREIGN KEY(AccountNumber)"
        " REFERENCES Accounts(AccountNumber));";
    if (!query.exec(prep_users_personal_data))
    {
        logger.log("Failed execution for Personal Data table.");
        logger.log("Error: " + query.lastError().text());
        dbConnection.rollback();
        query.finish();
        return false;
    }

    // Create Transaction_History table
    const QString prep_transaction_history =
        "CREATE TABLE Transaction_History (TransactionID INTEGER PRIMARY KEY AUTOINCREMENT,"
        " AccountNumber INTEGER, Date TEXT, Time TEXT, Amount REAL, FOREIGN KEY(AccountNumber)"
        " REFERENCES Accounts(AccountNumber));";
    if (!query.exec(prep_transaction_history))
    {
        logger.log("Failed execution for Transaction history table.");
        logger.log("Error: " + query.lastError().text());
        dbConnection.rollback();
        query.finish();
        return false;
    }

    // Commit transaction
    if (!dbConnection.commit())
    {
        logger.log("Failed to commit transaction for table creation.");
        dbConnection.rollback();
        query.finish();
        return false;
    }

    logger.log("Created all tables successfully.");
    query.finish();

    return true;
}

QJsonObject DatabaseManager::login(QJsonObject requestJson)
{
    QSqlDatabase dbConnection = QSqlDatabase::database(connectionName);
    QSqlQuery query(dbConnection);

    // Extract the username and password from the request JSON
    QString username = requestJson["username"].toString();
    QString password = requestJson["password"].toString();

    query.prepare("SELECT AccountNumber, Admin FROM Accounts "
                  "WHERE Username = :username AND Password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);
    if (!query.exec())
    {
        logger.log("Failed to execute query for login request.");
        query.finish();
        return QJsonObject();
    }

    QJsonObject responseJson;

    if (query.next())
    {
        // Login successful
        responseJson["loginSuccess"] = true;
        responseJson["accountNumber"] = query.value("AccountNumber").toLongLong();
        responseJson["isAdmin"] = query.value("Admin").toBool();
    }
    else
    {
        // Login failed
        responseJson["loginSuccess"] = false;
    }
    query.finish();

    return responseJson;
}

QJsonObject DatabaseManager::getAccountNumber(QJsonObject requestJson)
{
    QSqlDatabase dbConnection = QSqlDatabase::database(connectionName);
    QSqlQuery query(dbConnection);

    // Extract the username from the request JSON
    QString username = requestJson["username"].toString();

    query.prepare("SELECT AccountNumber FROM Accounts WHERE Username = :username");
    query.bindValue(":username", username);
    if (!query.exec())
    {
        logger.log("Failed to execute query for getAccountNumber request.");
        query.finish();
        return QJsonObject();
    }

    QJsonObject responseJson;

    if (query.next())
    {
        responseJson["accountNumber"] = query.value("AccountNumber").toLongLong();
        responseJson["userFound"] = true;
    }
    else
    {
        responseJson["userFound"] = false;
    }
    query.finish();

    return responseJson;
}

QJsonObject DatabaseManager::getAccountBalance(QJsonObject requestJson)
{
    QSqlDatabase dbConnection = QSqlDatabase::database(connectionName);
    QSqlQuery query(dbConnection);

    // Extract the account number from the request JSON
    qint64 accountNumber = requestJson["accountNumber"].toVariant().toLongLong();

    query.prepare("SELECT Balance FROM Users_Personal_Data WHERE AccountNumber = :accountNumber");
    query.bindValue(":accountNumber", accountNumber);

    QJsonObject responseJson;

    if (query.exec() && query.next())
    {
        responseJson["balance"] = query.value("Balance").toDouble();
        responseJson["accountFound"] = true;
    }
    else
    {
        responseJson["accountFound"] = false;
    }
    query.finish();

    return responseJson;
}

QJsonObject DatabaseManager::createNewAccount(QJsonObject requestJson)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    db.transaction();

    // Extract the necessary data from the request JSON
    bool isAdmin = requestJson["isAdmin"].toBool();
    QString username = requestJson["username"].toString();
    QString password = requestJson["password"].toString();
    QString name = requestJson["name"].toString();
    int age = requestJson["age"].toInt();
    double balance = 0.0;

    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(*) FROM Accounts WHERE Username = :username");
    checkQuery.bindValue(":username", username);

    QJsonObject responseJson;

    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0)
    {
        responseJson["createAccountSuccess"] = false;
        responseJson["errorMessage"] = "exists";
        db.rollback();
        checkQuery.finish();
        return responseJson;
    }

    QSqlQuery insertQuery(db);
    insertQuery.prepare("INSERT INTO Accounts (Username, Password, Admin) VALUES (:username, :password, :admin)");
    insertQuery.bindValue(":username", username);
    insertQuery.bindValue(":password", password);
    insertQuery.bindValue(":admin", isAdmin);

    if (!insertQuery.exec())
    {
        responseJson["createAccountSuccess"] = false;
        responseJson["errorMessage"] = "failed";
        db.rollback();
        insertQuery.finish();
        return responseJson;
    }

    qint64 accountNumber = insertQuery.lastInsertId().toLongLong();

    QSqlQuery personalDataQuery(db);
    personalDataQuery.prepare("INSERT INTO Users_Personal_Data (AccountNumber, Name, Age, Balance) "
                              "VALUES (:accountNumber, :name, :age, :balance)");
    personalDataQuery.bindValue(":accountNumber", accountNumber);
    personalDataQuery.bindValue(":name", name);
    personalDataQuery.bindValue(":age", age);
    personalDataQuery.bindValue(":balance", balance);

    if (!personalDataQuery.exec())
    {
        responseJson["createAccountSuccess"] = false;
        responseJson["errorMessage"] = "failed";
        db.rollback();
        personalDataQuery.finish();
        return responseJson;
    }

    db.commit();
    responseJson["createAccountSuccess"] = true;
    responseJson["accountNumber"] = accountNumber;
    checkQuery.finish();
    insertQuery.finish();
    personalDataQuery.finish();
    return responseJson;
}

QJsonObject DatabaseManager::deleteAccount(QJsonObject requestJson)
{
    QSqlDatabase dbConnection = QSqlDatabase::database(connectionName);

    // Extract the account number from the request JSON
    qint64 accountNumber = requestJson["accountNumber"].toVariant().toLongLong();

    // Start a transaction
    if (!dbConnection.transaction())
    {
        logger.log("Failed to start transaction.");
        return QJsonObject();
    }

    QSqlQuery deleteQuery(dbConnection);
    deleteQuery.prepare("DELETE FROM Accounts WHERE AccountNumber = :accountNumber");
    deleteQuery.bindValue(":accountNumber", accountNumber);

    QJsonObject responseJson;

    if (!deleteQuery.exec())
    {
        logger.log("Failed to delete account from Accounts table.");
        dbConnection.rollback();
        responseJson["deleteAccountSuccess"] = false;
        deleteQuery.finish();
        return responseJson;
    }

    QSqlQuery deletePersonalDataQuery(dbConnection);
    deletePersonalDataQuery.prepare("DELETE FROM Users_Personal_Data WHERE AccountNumber = :accountNumber");
    deletePersonalDataQuery.bindValue(":accountNumber", accountNumber);

    if (!deletePersonalDataQuery.exec())
    {
        logger.log("Failed to delete account from Users_Personal_Data table.");
        dbConnection.rollback();
        responseJson["deleteAccountSuccess"] = false;
        deletePersonalDataQuery.finish();
        return responseJson;
    }

    QSqlQuery deleteTransactionQuery(dbConnection);
    deleteTransactionQuery.prepare("DELETE FROM Transaction_History WHERE AccountNumber = :accountNumber");
    deleteTransactionQuery.bindValue(":accountNumber", accountNumber);

    if(!deleteTransactionQuery.exec())
    {
        logger.log("Failed to delete transaction history for the account.");
        dbConnection.rollback();
        responseJson["deleteAccountSuccess"] = false;
        deleteTransactionQuery.finish();
        return responseJson;
    }

    // If all delete operations succeed, commit the transaction
    if (!dbConnection.commit())
    {
        logger.log("Failed to commit transaction.");
        responseJson["deleteAccountSuccess"] = false;
        return responseJson;
    }
    deleteQuery.finish();
    deletePersonalDataQuery.finish();
    deleteTransactionQuery.finish();
    responseJson["deleteAccountSuccess"] = true;

    return responseJson;
}

QJsonObject DatabaseManager::fetchAllUserData()
{
    QSqlDatabase db = QSqlDatabase::database(connectionName);

    QSqlQuery fetchAllUserDataQuery(db);
    fetchAllUserDataQuery.prepare("SELECT Accounts.AccountNumber, Accounts.Username, Users_Personal_Data.Name, "
                                  "Users_Personal_Data.Balance, Users_Personal_Data.Age "
                                  "FROM Accounts JOIN Users_Personal_Data "
                                  "ON Accounts.AccountNumber = Users_Personal_Data.AccountNumber");

    QJsonObject responseJson;

    if (!fetchAllUserDataQuery.exec())
    {
        responseJson["fetchUserDataSuccess"] = false;
        responseJson["errorMessage"] = "failed";
        fetchAllUserDataQuery.finish();
        return responseJson;
    }

    // Create a JSON array to store user data
    QJsonArray userDataArray;

    while (fetchAllUserDataQuery.next())
    {
        QJsonObject userData;
        userData["AccountNumber"] = fetchAllUserDataQuery.value("AccountNumber").toLongLong();
        userData["Username"] = fetchAllUserDataQuery.value("Username").toString();
        userData["Name"] = fetchAllUserDataQuery.value("Name").toString();
        userData["Balance"] = fetchAllUserDataQuery.value("Balance").toDouble();
        userData["Age"] = fetchAllUserDataQuery.value("Age").toInt();

        userDataArray.append(userData);
    }

    responseJson["fetchUserDataSuccess"] = true;
    responseJson["userData"] = userDataArray;
    fetchAllUserDataQuery.finish();

    return responseJson;
}

QJsonObject DatabaseManager::makeTransaction(QJsonObject requestJson)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    db.transaction();

    // Extract the necessary data from the request JSON
    qint64 accountNumber = requestJson["accountNumber"].toVariant().toLongLong();
    double amount = requestJson["amount"].toDouble();

    // Check if the balance is sufficient
    QJsonObject balanceObj = getAccountBalance(requestJson);

    double currentBalance = balanceObj["balance"].toDouble();

    QJsonObject responseJson;

    if (currentBalance < 0 || currentBalance + amount < 0)
    {
        responseJson["transactionSuccess"] = false;
        responseJson["errorMessage"] = "Insufficient balance";
        db.rollback();
        return responseJson;
    }

    // Update the balance
    double newBalance = currentBalance + amount;  // Reverse the logic here
    QSqlQuery updateBalanceQuery(db);
    updateBalanceQuery.prepare("UPDATE Users_Personal_Data SET Balance = :balance WHERE AccountNumber = :accountNumber");
    updateBalanceQuery.bindValue(":balance", newBalance);
    updateBalanceQuery.bindValue(":accountNumber", accountNumber);

    if (!updateBalanceQuery.exec())
    {
        responseJson["transactionSuccess"] = false;
        responseJson["errorMessage"] = "Failed to update balance";
        db.rollback();
        updateBalanceQuery.finish();
        return responseJson;
    }

    // Log the transaction in the Transaction_History table
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDate = currentDateTime.toString("dd-MM-yyyy");
    QString formattedTime = currentDateTime.toString("hh:mm:ss");

    QSqlQuery logTransactionQuery(db);
    logTransactionQuery.prepare("INSERT INTO Transaction_History (AccountNumber, Date, Time, Amount) "
                                "VALUES (:accountNumber, :date, :time, :amount)");
    logTransactionQuery.bindValue(":accountNumber", accountNumber);
    logTransactionQuery.bindValue(":date", formattedDate);
    logTransactionQuery.bindValue(":time", formattedTime);
    logTransactionQuery.bindValue(":amount", amount);

    if (!logTransactionQuery.exec())
    {
        responseJson["transactionSuccess"] = false;
        responseJson["errorMessage"] = "Failed to log transaction";
        db.rollback();
        logTransactionQuery.finish();
        return responseJson;
    }

    db.commit();
    responseJson["transactionSuccess"] = true;
    responseJson["newBalance"] = newBalance;
    updateBalanceQuery.finish();
    logTransactionQuery.finish();
    return responseJson;
}

QJsonObject DatabaseManager::makeTransfer(QJsonObject requestJson)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    db.transaction();

    // Extract the necessary data from the request JSON
    qint64 fromAccountNumber = requestJson["fromAccountNumber"].toVariant().toLongLong();
    qint64 toAccountNumber = requestJson["toAccountNumber"].toVariant().toLongLong();
    double amount = requestJson["amount"].toDouble();

    // Create a new JSON object for the 'from' account balance request
    QJsonObject fromBalanceRequest;
    fromBalanceRequest["accountNumber"] = fromAccountNumber;

    // Check if the 'from' account has sufficient balance
    QJsonObject fromBalanceObj = getAccountBalance(fromBalanceRequest);
    double fromAccountBalance = fromBalanceObj["balance"].toDouble();

    QJsonObject responseJson;

    if (fromAccountBalance < 0 || fromAccountBalance - amount < 0)
    {
        responseJson["transferSuccess"] = false;
        responseJson["errorMessage"] = "Insufficient balance for the transfer";
        db.rollback();
        return responseJson;
    }

    // Update the 'from' and 'to' account balances
    double newFromBalance = fromAccountBalance - amount;

    // Create a new JSON object for the 'to' account balance request
    QJsonObject toBalanceRequest;
    toBalanceRequest["accountNumber"] = toAccountNumber;

    QJsonObject toBalanceObj = getAccountBalance(toBalanceRequest);
    double newToBalance = toBalanceObj["balance"].toDouble() + amount;

    QSqlQuery updateBalanceQuery(db);
    updateBalanceQuery.prepare("UPDATE Users_Personal_Data SET Balance = :balance WHERE AccountNumber = :accountNumber");
    updateBalanceQuery.bindValue(":balance", newFromBalance);
    updateBalanceQuery.bindValue(":accountNumber", fromAccountNumber);

    if (!updateBalanceQuery.exec())
    {
        responseJson["transferSuccess"] = false;
        responseJson["errorMessage"] = "Failed to update 'from' account balance";
        db.rollback();
        updateBalanceQuery.finish();
        return responseJson;
    }

    updateBalanceQuery.bindValue(":balance", newToBalance);
    updateBalanceQuery.bindValue(":accountNumber", toAccountNumber);

    if (!updateBalanceQuery.exec())
    {
        responseJson["transferSuccess"] = false;
        responseJson["errorMessage"] = "Failed to update 'to' account balance";
        db.rollback();
        updateBalanceQuery.finish();
        return responseJson;
    }

    // Log the transfer in the Transaction_History table for both 'from' and 'to' accounts
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDate = currentDateTime.toString("dd-MM-yyyy");
    QString formattedTime = currentDateTime.toString("hh:mm:ss");

    QSqlQuery logTransactionQuery(db);
    logTransactionQuery.prepare("INSERT INTO Transaction_History (AccountNumber, Date, Time, Amount) "
                                "VALUES (:accountNumber, :date, :time, :amount)");
    logTransactionQuery.bindValue(":accountNumber", fromAccountNumber);
    logTransactionQuery.bindValue(":date", formattedDate);
    logTransactionQuery.bindValue(":time", formattedTime);
    logTransactionQuery.bindValue(":amount", -amount); // Negative amount for 'from' account

    if (!logTransactionQuery.exec())
    {
        responseJson["transferSuccess"] = false;
        responseJson["errorMessage"] = "Failed to log 'from' account transaction";
        db.rollback();
        logTransactionQuery.finish();
        return responseJson;
    }

    logTransactionQuery.bindValue(":accountNumber", toAccountNumber);
    logTransactionQuery.bindValue(":amount", amount); // Positive amount for 'to' account

    if (!logTransactionQuery.exec())
    {
        responseJson["transferSuccess"] = false;
        responseJson["errorMessage"] = "Failed to log 'to' account transaction";
        db.rollback();
        logTransactionQuery.finish();
        return responseJson;
    }

    db.commit();
    responseJson["transferSuccess"] = true;
    responseJson["newFromBalance"] = newFromBalance;
    responseJson["newToBalance"] = newToBalance;
    updateBalanceQuery.finish();
    logTransactionQuery.finish();

    return responseJson;
}

QJsonObject DatabaseManager::viewTransactionHistory(QJsonObject requestJson)
{
    QSqlDatabase dbConnection = QSqlDatabase::database(connectionName);
    QSqlQuery query(dbConnection);

    // Extract the account number from the request JSON
    qint64 accountNumber = requestJson["accountNumber"].toVariant().toLongLong();

    query.prepare("SELECT TransactionID, Date, Time, Amount FROM Transaction_History "
                  "WHERE AccountNumber = :accountNumber ORDER BY Date DESC, Time DESC");

    query.bindValue(":accountNumber", accountNumber);

    QJsonObject responseJson;
    QJsonArray transactionHistoryArray;

    if (query.exec())
    {
        while (query.next())
        {
            QJsonObject transactionObj;
            transactionObj["TransactionID"] = query.value("TransactionID").toLongLong();
            transactionObj["Date"] = query.value("Date").toString();
            transactionObj["Time"] = query.value("Time").toString();
            transactionObj["Amount"] = query.value("Amount").toDouble();

            transactionHistoryArray.append(transactionObj);
        }
    }

    responseJson["transactionHistory"] = transactionHistoryArray;
    responseJson["viewTransactionHistorySuccess"] = true;
    query.finish();

    return responseJson;
}

QJsonObject DatabaseManager::updateUserData(QJsonObject requestJson)
{
    QSqlDatabase db = QSqlDatabase::database(connectionName);

    // Extract the necessary data from the request JSON
    QString username = requestJson["username"].toString();
    QString name = requestJson["name"].toString();
    QString password = requestJson["password"].toString();

    // Check if the account exists and get the account number
    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT AccountNumber FROM Accounts WHERE Username = :username");
    checkQuery.bindValue(":username", username);

    QJsonObject responseJson;

    if (checkQuery.exec() && checkQuery.next())
    {
        qint64 accountNumber = checkQuery.value(0).toLongLong();

        // The account exists, proceed with the update
        if (!password.isEmpty())
        {
            QSqlQuery updateQuery(db);
            updateQuery.prepare("UPDATE Accounts SET Password = :password WHERE Username = :username");
            updateQuery.bindValue(":username", username);
            updateQuery.bindValue(":password", password);
            if (!updateQuery.exec())
            {
                responseJson["updateSuccess"] = false;
                responseJson["errorMessage"] = "Failed to update password";
                checkQuery.finish();
                return responseJson;
            }
        }

        if (!name.isEmpty())
        {
            QSqlQuery updateQuery(db);
            updateQuery.prepare("UPDATE Users_Personal_Data SET Name = :name WHERE AccountNumber = :accountNumber");
            updateQuery.bindValue(":accountNumber", accountNumber);
            updateQuery.bindValue(":name", name);
            if (!updateQuery.exec())
            {
                responseJson["updateSuccess"] = false;
                responseJson["errorMessage"] = "Failed to update name";
                updateQuery.finish();
                return responseJson;
            }
        }

        responseJson["updateSuccess"] = true;
    }
    else
    {
        // The account does not exist
        responseJson["updateSuccess"] = false;
        responseJson["errorMessage"] = "Account not found";
    }
    checkQuery.finish();

    return responseJson;
}
