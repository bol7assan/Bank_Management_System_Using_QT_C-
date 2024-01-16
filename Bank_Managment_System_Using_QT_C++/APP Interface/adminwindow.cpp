#include "adminwindow.h"
#include "ui_adminwindow.h"

const QRegularExpression AdminWindow::usernameRegex("^[a-zA-Z0-9_]*$");
const QRegularExpression AdminWindow::passwordRegex("\\s");

AdminWindow::AdminWindow(QWidget *parent, qint64 accountNumber, QTcpSocket *socket)
    : QMainWindow(parent), ui(new Ui::AdminWindow), socket(socket), accountNumber(accountNumber)
{
    ui->setupUi(this);
    setWindowTitle("Admin Window - Account Number: " + QString::number(accountNumber));

    // Set the Qt::WA_DeleteOnClose attribute to ensure the destructor is called on close
    setAttribute(Qt::WA_DeleteOnClose);
    connect(socket, &QTcpSocket::readyRead, this, &AdminWindow::readyRead);
    qDebug() <<"Constructed Admin Window.";
}

AdminWindow::~AdminWindow()
{
    delete ui;
    // Emit the finished signal when the window is closed
    emit finished();
    qDebug() << "Destroyed Admin Window.";
}

void AdminWindow::readyRead()
{
    // read data from socket
    responseData.append(socket->readAll());

    // Check if we have a complete message (newline delimited) only happens when receiving a very large response
    if (!responseData.endsWith('\n'))
    {
        return;  // If not, return and wait for more data
    }

    // Try to parse the JSON document
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    // if we get here hooray :D its a kinda valid json after a ton of buffering maybe
    responseData.clear();

    //qDebug() << "Raw Response Data:" << responseData;

    // Check if the response is a valid JSON object
    if (!jsonResponse.isObject())
    {
        qDebug() << "Invalid JSON response from the server.";
        return;
    }

    //qDebug() << "Raw Response Data:" << responseData;

    // Handle the response based on the request ID
    QJsonObject responseObject = jsonResponse.object();
    int responseId = responseObject["responseId"].toInt();

    switch (responseId)
    {
    case 1:
        handleGetAccountNumberResponse(responseObject);
        break;
    case 2:
        handleViewAccountBalanceResponse(responseObject);
        break;
    case 3:
        handleCreateNewAccountResponse(responseObject);
        break;
    case 4:
        handleDeleteAccountResponse(responseObject);
        break;
    case 5:
        handleFetchAllUserDataResponse(responseObject);
        break;
    case 8:
        handleViewTransactionHistoryResponse(responseObject);
        break;
    case 9:
        handleUpdateAccountResponse(responseObject);
        break;
    default:
        qDebug() << "Unknown responseId ID: " << responseId;
        break;
    }
    // qDebug() << responseData;
}


void AdminWindow::on_pushButton_get_account_number_clicked()
{
    ui->label_error->clear();
    // Request ID for Get Account Number
    quint8 requestId = 1;
    // Get the username from the UI
    QString username = ui->lineEdit_username->text();
    // Check if Username and Password fields are not empty
    if (username.isEmpty())
    {
        qDebug() << "Warning: Please fill in the username and password.";
        ui->label_error->setText("Can't be empty.");
        return;
    }

    // Check if the Username field contains only alphanumeric characters and underscores
    if (!username.contains(usernameRegex))
    {
        qDebug() << "Warning: Username must only contain alphanumeric characters.";
        ui->label_error->setText("Can't have any special chars only Underscores");
        return;
    }

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);
    requestObject["username"] = username;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
}

void AdminWindow::handleGetAccountNumberResponse(const QJsonObject &responseObject)
{
    ui->label_error->clear();
    ui->label_view_account_number->setText("Account Number: ");
    qint64 accountnumber = responseObject["accountNumber"].toVariant().toLongLong();
    bool userFound = responseObject["userFound"].toBool();
    if (userFound)
    {
        ui->label_view_account_number->setText("Account Number: "
                                               + QString::number(accountnumber));
        ui->label_error->setText("Success :)");
        qDebug() << "User found. Account Number: " << accountnumber;
    }
    else
    {
        ui->label_error->setText("User Not Found!");
        qDebug() << "User not found.";
    }
}

void AdminWindow::on_pbn_view_balance_clicked()
{
    ui->label_error_viewbalance->clear();
    // Request ID for View Account Balance
    quint8 requestId = 2;
    // Get the account number from the UI
    qint64 accountNumber = ui->lnedit_accountnumber->text().toLongLong();

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);
    requestObject["accountNumber"] = accountNumber;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
}

void AdminWindow::handleViewAccountBalanceResponse(const QJsonObject &responseObject)
{
    ui->label_error_viewbalance->clear();
    ui->lbl_viewbalance->setText("Balance: ");
    double accountBalance = responseObject["balance"].toDouble();
    bool viewBalanceSuccess = responseObject["accountFound"].toBool();

    if (viewBalanceSuccess)
    {
        ui->lbl_viewbalance->setText("Balance: $" + QString::number(accountBalance));
        ui->label_error_viewbalance->setText("Success :)");
        qDebug() << "View Account Balance successful. Balance: $" << accountBalance;
    }
    else
    {
        ui->label_error_viewbalance->setText("Failed to view Account Balance!");
        qDebug() << "Failed to view Account Balance.";
    }
}

void AdminWindow::on_pbn_create_new_account_clicked()
{
    ui->lbl_error_create->clear();

    // Request ID for Create Account
    quint8 requestId = 3;

    // Get the necessary information from the UI
    QString username = ui->lnedit_username_input->text();
    QString password = ui->lnedit_password_input->text();
    QString name = ui->lnedit_name_input->text();
    quint8 age = ui->lnedit_age_input->text().toUInt();
    bool isAdmin = ui->chkbox_admin->isChecked();

    // Check if any of the required fields are empty
    if (username.isEmpty() ||
        password.isEmpty() ||
        name.isEmpty())
    {
        qDebug() << "Warning: Please fill in all the required fields.";
        ui->lbl_error_create->setText("All fields are required.");
        return;
    }

    // Check if the Username field contains only alphanumeric characters and underscores
    if (!username.contains(usernameRegex))
    {
        qDebug() << "Warning: Username must only contain alphanumeric characters.";
        ui->lbl_error_create->setText("Username can only have alphanumeric characters.");
        return;
    }

    // Check if the Password field contains whitespace
    if (password.contains(passwordRegex))
    {
        qDebug() << "Warning: Password must not contain whitespace.";
        ui->lbl_error_create->setText("Password must not contain whitespace.");
        return;
    }

    // Check if the age is between 18 and 120
    if (age < 18 || age > 120)
    {
        qDebug() << "Warning: Age must be between 18 and 120.";
        ui->lbl_error_create->setText("Age must be between 18 and 120.");
        return;
    }

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);
    requestObject["username"] = username;
    requestObject["password"] = password;
    requestObject["name"] = name;
    requestObject["age"] = age;
    requestObject["isAdmin"] = isAdmin;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());

}

void AdminWindow::handleCreateNewAccountResponse(const QJsonObject &responseObject)
{
    ui->lbl_error_create->clear();

    bool createAccountSuccess = responseObject["createAccountSuccess"].toBool();
    qint64 accountNumber = responseObject["accountNumber"].toVariant().toLongLong();

    if (createAccountSuccess)
    {
        ui->lbl_error_create->setText("Account created. Account Number: "
                                               + QString::number(accountNumber));
        qDebug() << "Create Account successful. Account Number: " << accountNumber;
    }
    else
    {
        QString errorMessage = responseObject["errorMessage"].toString();
        ui->lbl_error_create->setText("Failed Error: " + errorMessage);
        qDebug() << "Failed to create account. Error: " << errorMessage;
    }
}

void AdminWindow::on_pbn_delete_account_clicked()
{
    ui->lbl_error_delete->clear();

    if (ui->chkbox_sure->isChecked())
    {
        // Request ID for Delete Account
        quint8 requestId = 4;

        // Get the account number from the UI
        qint64 accountNumber = ui->lnedit_delete_account_number->text().toLongLong();

        // Construct the request JSON object
        QJsonObject requestObject;
        requestObject["requestId"] = static_cast<int>(requestId);
        requestObject["accountNumber"] = accountNumber;

        // Convert the JSON object to a JSON document
        QJsonDocument jsonRequest(requestObject);

        // Send the request to the server
        socket->write(jsonRequest.toJson());
    }
    else
    {
        ui->lbl_error_delete->setText("Please check the confirmation box to delete the account.");
        qDebug() << "Deletion canceled. To delete the account, please check the confirmation box.";
    }
}

void AdminWindow::handleDeleteAccountResponse(const QJsonObject &responseObject)
{
    ui->lbl_error_delete->clear();

    bool deleteAccountSuccess = responseObject["deleteAccountSuccess"].toBool();

    if (deleteAccountSuccess)
    {
        ui->lbl_error_delete->setText("Account deleted successfully.");
        qDebug() << "Delete Account successful.";
    }
    else
    {
        ui->lbl_error_delete->setText("Failed to delete account.");
        qDebug() << "Failed to delete account.";
    }
}

void AdminWindow::handleFetchAllUserDataResponse(const QJsonObject &responseObject)
{
    ui->label_error->clear();

    bool fetchUserDataSuccess = responseObject["fetchUserDataSuccess"].toBool();

    if (fetchUserDataSuccess)
    {
        // Clear existing data in tbl_view_database
        ui->tbl_view_database->clearContents();
        ui->tbl_view_database->setRowCount(0);

        // Get the user data array from the response
        QJsonArray userDataArray = responseObject["userData"].toArray();

        // Populate tbl_view_database with user data
        int row = 0;
        for (const auto &userDataValue : userDataArray)
        {
            QJsonObject userData = userDataValue.toObject();

            ui->tbl_view_database->insertRow(row);
            ui->tbl_view_database->setItem(row, 0, new QTableWidgetItem(QString::number(userData["AccountNumber"].toInt())));
            ui->tbl_view_database->setItem(row, 1, new QTableWidgetItem(userData["Username"].toString()));
            ui->tbl_view_database->setItem(row, 2, new QTableWidgetItem(userData["Name"].toString()));
            ui->tbl_view_database->setItem(row, 3, new QTableWidgetItem(QString::number(userData["Balance"].toDouble())));
            ui->tbl_view_database->setItem(row, 4, new QTableWidgetItem(QString::number(userData["Age"].toInt())));

            // Debugging statements
            qDebug() << "AccountNumber:" << QString::number(userData["AccountNumber"].toInt());
            qDebug() << "Username:" << userData["Username"].toString();
            qDebug() << "Name:" << userData["Name"].toString();
            qDebug() << "Balance:" << QString::number(userData["Balance"].toDouble());
            qDebug() << "Age:" << QString::number(userData["Age"].toInt());

            row++;
        }
    }
    else
    {
        ui->label_error->setText("Failed to fetch user data.");
        qDebug() << "Failed to fetch user data.";
    }
}

void AdminWindow::on_pbn_view_database_clicked()
{
    ui->lbl_view_database_error->clear();

    // Request ID for Fetch All User Data
    quint8 requestId = 5;

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
}

void AdminWindow::on_pbn_view_transaction_history_clicked()
{
    // Get the account number to view transaction history
    qint64 accountNumber=ui->lnedit_act_number_transaction_history->text().toInt();

    // Validate the accountNumber
    if (accountNumber <= 0) {
        // Display an error message
        ui->lbl_err_transaction_history->setText("Invalid Account Number. Please enter a valid number.");
        return;
    }

    // Clear any previous error messages
    ui->lbl_err_transaction_history->clear();

    // Request ID for View Transaction History
    quint8 requestId = 8;

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);
    requestObject["accountNumber"] = accountNumber;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
}

void AdminWindow::handleViewTransactionHistoryResponse(const QJsonObject &responseObject)
{
    ui->lbl_err_transaction_history->clear();

    bool viewTransactionHistorySuccess = responseObject["viewTransactionHistorySuccess"].toBool();

    if (viewTransactionHistorySuccess)
    {
        // Clear existing data in tbl_transaction_history
        ui->tbl_transaction_history->clearContents();
        ui->tbl_transaction_history->setRowCount(0);

        // Get the transaction history array from the response
        QJsonArray transactionHistoryArray = responseObject["transactionHistory"].toArray();

        qint32 maxRows = ui->lnedit_count->text().isEmpty() ?
                          transactionHistoryArray.size() :
                          ui->lnedit_count->text().toInt();

        // Populate tbl_transaction_history with transaction history data
        int row = 0;
        for (const auto &transactionDataValue : transactionHistoryArray)
        {
            // Stop the loop if reached the maximum number of rows
            if (row >= maxRows)
                break;

            QJsonObject transactionData = transactionDataValue.toObject();

            ui->tbl_transaction_history->insertRow(row);
            ui->tbl_transaction_history->setItem(row, 0, new QTableWidgetItem(QString::number(transactionData["TransactionID"].toVariant().toLongLong())));
            ui->tbl_transaction_history->setItem(row, 1, new QTableWidgetItem(QString::number(transactionData["Amount"].toDouble())));
            ui->tbl_transaction_history->setItem(row, 2, new QTableWidgetItem(transactionData["Date"].toString()));
            ui->tbl_transaction_history->setItem(row, 3, new QTableWidgetItem(transactionData["Time"].toString()));

            // Debugging statements
            // qDebug() << "TransactionID:" << QString::number(transactionData["TransactionID"].toVariant().toLongLong());
            // qDebug() << "Amount:" << QString::number(transactionData["Amount"].toDouble());
            // qDebug() << "Date:" << transactionData["Date"].toString();
            // qDebug() << "Time:" << transactionData["Time"].toString();

            row++;
        }

        qDebug() << "View Transaction History successful.";
    }
    else
    {
        ui->lbl_err_transaction_history->setText("Failed to view transaction history.");
        qDebug() << "Failed to view Transaction History.";
    }
}
void AdminWindow::on_pbn_update_account_clicked()
{
    // Request ID for Update Account
    quint8 requestId = 9;

    // Get the necessary information from the UI
    QString username = ui->lnedit_username_input->text();
    QString password = ui->lnedit_password_input->text();
    QString name = ui->lnedit_name_input->text();

    // Check if username is provided and matches the regex
    if (username.isEmpty() || !usernameRegex.match(username).hasMatch())
    {
        qDebug() << "Warning: Please provide a valid username.";
        ui->lbl_error_create->setText("Please provide a valid username.");
        return;
    }

    // Check if password is valid
    if (!password.isEmpty() && passwordRegex.match(password).hasMatch())
    {
        qDebug() << "Warning: Password cannot contain whitespace.";
        ui->lbl_error_create->setText("Password cannot contain whitespace.");
        return;
    }

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);
    requestObject["username"] = username;

    // Only add the fields to the JSON object if they are not empty
    if (!password.isEmpty())
        requestObject["password"] = password;
    if (!name.isEmpty())
        requestObject["name"] = name;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
}

void AdminWindow::handleUpdateAccountResponse(const QJsonObject &responseObject)
{
    bool updateSuccess = responseObject["updateSuccess"].toBool();

    if (updateSuccess)
    {
        ui->lbl_error_create->setText("Update Account successful.");
        qDebug() << "Update Account successful.";
    }
    else
    {
        QString errorMessage = responseObject["errorMessage"].toString();
        ui->lbl_error_create->setText(errorMessage);
        qDebug() << "Failed to update account. Error: " << errorMessage;
    }
}


