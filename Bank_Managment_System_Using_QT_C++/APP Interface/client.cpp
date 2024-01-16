#include "client.h"
#include "ui_client.h"
#include "adminwindow.h"
#include "userwindow.h"

// Regular expressions for username and password validation
const QRegularExpression client::usernameRegex("^[a-zA-Z0-9_]*$");
const QRegularExpression client::passwordRegex("\\s");

// Constructor
client::client(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::client),
    socket(new QTcpSocket(this)),accountNumber(0)
{
    // Setup the UI
    ui->setupUi(this);

    ui->tabWidget->hide();

    // Set the window elements
    setWindowTitle("Bank APP");
    ui->pushButton_logout->hide();

    // Attempt to connect to the server
    socket->connectToHost("localhost", 54321);

    // Connect the readyRead signal to the readyRead slot
    connect(socket, &QTcpSocket::readyRead, this, &client::readyRead);
}

// Destructor
client::~client()
{
    // Flush the socket
    socket->flush();
    // Delete the UI
    delete ui;
}

// Slot for handling incoming data from the server
void client::readyRead()
{
    // Read the response from the server
    QByteArray responseData = socket->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    // Check if the response is a valid JSON object
    if (!jsonResponse.isObject())
    {
        qDebug() << "Invalid JSON response from the server.";
        return;
    }

    // Handle the response based on the request ID
    QJsonObject responseObject = jsonResponse.object();
    int responseId = responseObject["responseId"].toInt();

    switch (responseId)
    {
    case 0:
        handleLoginResponse(responseObject);
        break;
    case 1:
        handleGetAccountNumberResponse(responseObject);
        break;
    case 2:
        handleViewAccountBalanceResponse(responseObject);
        break;
    case 3:
        handleCreateNewAccountResponse(responseObject);
        break;
    case 5:
        handleFetchAllUserDataResponse(responseObject);
        break;
    case 6:
        handleMakeTransactionResponse(responseObject);
        break;
    case 7:
        handleMakeTransferResponse(responseObject);
        break;
    case 8:
        handleViewTransactionHistoryResponse(responseObject);
        break;
    case 10:
        adminHandleViewAccountBalanceResponse(responseObject);
        break;
    case 11:
        adminHandleViewTransactionHistoryResponse(responseObject);
        break;
    default:
        qDebug() << "Unknown responseId ID: " << responseId;
        break;
    }
    // qDebug() << responseData;
}

// Slot for handling login button click
void client::on_pushButton_login_clicked()
{
    // Request ID for login
    quint8 requestId = 0;
    // Get the username and password from the UI
    QString username = ui->lineEdit_Username->text();
    QString password = ui->lineEdit_Password->text();

    // Check if Username and Password fields are not empty
    if (username.isEmpty() || password.isEmpty())
    {
        qDebug() << "Warning: Please fill in the username and password.";
        QMessageBox::warning(nullptr, "Warning", "Please fill in the username and password.", QMessageBox::Ok);
        return;
    }

    // Check if the Username field contains only alphanumeric characters and underscores
    if (!username.contains(usernameRegex))
    {
        qDebug() << "Warning: Username must only contain alphanumeric characters.";
        QMessageBox::warning(nullptr, "Warning", "Username must only contain alphanumeric characters.", QMessageBox::Ok);
        return;
    }

    // Check if the password contains spaces
    if (password.contains(passwordRegex))
    {
        qDebug() << "Warning: Password must not contain spaces.";
        QMessageBox::warning(nullptr, "Warning", "Password must not contain spaces.", QMessageBox::Ok);
        return;
    }

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);
    requestObject["username"] = username;
    requestObject["password"] = password;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
}

// Function to handle login response from the server
void client::handleLoginResponse(const QJsonObject &responseObject)
{
    // Extract information from the response
    bool loginSuccess = responseObject["loginSuccess"].toBool();
    accountNumber = responseObject["accountNumber"].toVariant().toLongLong();
    bool isAdmin = responseObject["isAdmin"].toBool();
    ui->lineEdit_Username->clear();
    ui->lineEdit_Password->clear();
    if (loginSuccess)
    {
        ui->label_Username->hide();
        ui->label_Password->hide();
        ui->lineEdit_Username->hide();
        ui->lineEdit_Password->hide();
        ui->pushButton_login->hide();
        ui->pushButton_logout->show();
        ui->tabWidget->show();
        // enable and disable tabs based on user type
        if (isAdmin)
        {
            // Enable the admin tab and disable the user tab
            ui->tabWidget->setTabEnabled(0, true); // Admin tab
            ui->tabWidget->setTabEnabled(1, false); // User tab
            ui->tabWidget->setCurrentIndex(0);
        }
        else
        {
            // Enable the user tab and disable the admin tab
            ui->tabWidget->setTabEnabled(0, false); // Admin tab
            ui->tabWidget->setTabEnabled(1, true); // User tab
            ui->tabWidget->setCurrentIndex(1);
        }
    }
    else
    {
        // Display a login failed message
        QMessageBox::warning(this, "Login Failed", "Invalid username or password. Please try again.", QMessageBox::Ok);
    }
}

void client::on_pushButton_logout_clicked()
{
    ui->label_Username->show();
    ui->label_Password->show();
    ui->lineEdit_Username->show();
    ui->lineEdit_Password->show();
    ui->pushButton_login->show();
    ui->pushButton_logout->hide();
    ui->tabWidget->setTabEnabled(0, false); // Disable Admin tab
    ui->tabWidget->setTabEnabled(1, false); // Disable User tab
    ui->tabWidget->hide();
    ui->label_account_number_2->clear();
}

void client::on_pushButton_get_account_number_4_clicked()
{
    ui->label_account_number_2->setText("Number: "
                                      + QString::number(accountNumber));
}

void client::on_pbn_view_balance_4_clicked()
{
    // Request ID for View Account Balance
    quint8 requestId = 2;

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);
    requestObject["accountNumber"] = accountNumber;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
}

void client::handleViewAccountBalanceResponse(const QJsonObject &responseObject)
{
    ui->label_view_balance_2->setText("Balance: ");
    double accountBalance = responseObject["balance"].toDouble();
    bool viewBalanceSuccess = responseObject["accountFound"].toBool();

    if (viewBalanceSuccess)
    {
        ui->label_view_balance_2->setText("Balance: $" + QString::number(accountBalance));
        qDebug() << "View Account Balance successful. Balance: $" << accountBalance;
    }
    else
    {
        ui->label_view_balance_2->setText("Failed to view Account Balance!");
        qDebug() << "Failed to view Account Balance.";
    }
}

void client::on_pbn_make_trasnaction_2_clicked()
{
    QString amountString = ui->lnedit_amount_2->text();
    bool conversionOk;
    double amount = amountString.toDouble(&conversionOk);

    if (!conversionOk || amountString.isEmpty()) {
        // Display an error message in lbl_transaction_error
        ui->lbl_transaction_error_2->setText("Invalid amount entered. Please enter a valid number.");
        return;
    }

    // Clear any previous error messages
    ui->lbl_transaction_error_2->clear();

    // Request ID for Make Transaction
    quint8 requestId = 6;

    // Create a JSON object for the transaction request
    QJsonObject transactionRequest;
    transactionRequest["requestId"] = static_cast<int>(requestId);
    transactionRequest["accountNumber"] = accountNumber;
    transactionRequest["amount"] = amount;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(transactionRequest);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
    socket->flush();
}


void client::handleMakeTransactionResponse(const QJsonObject &responseObject)
{
    ui->lbl_transaction_error_2->clear();
    double newBalance = responseObject["newBalance"].toDouble();
    bool transactionSuccess = responseObject["transactionSuccess"].toBool();

    if (transactionSuccess)
    {
        ui->lbl_transaction_error_2->setText("Transaction successful :)");
        qDebug() << "Transaction successful. New balance: $" << newBalance;
    }
    else
    {
        QString errorMessage = responseObject["errorMessage"].toString();
        ui->lbl_transaction_error_2->setText("Transaction failed: " + errorMessage);
        qDebug() << "Transaction failed: " << errorMessage;
    }
}

void client::on_pbn_mk_transfer_2_clicked()
{
    // Get the account number to transfer to
    qint64 toAccountNumber = ui->lnedit_to_accountnumber_2->text().toLongLong();

    // Validate the toAccountNumber
    if (toAccountNumber <= 0) {
        ui->lbl_mk_trnsf_err_2->setText("Invalid 'To Account Number'. Please enter a valid number.");
        return;
    }

    // Get the transfer amount
    QString amountString = ui->lnedit_trnsfr_amount_2->text();
    bool conversionOk;
    double amount = amountString.toDouble(&conversionOk);

    if (!conversionOk || amountString.isEmpty() || amount < 0) {
        ui->lbl_mk_trnsf_err_2->setText("Invalid transfer amount. Please enter a valid positive number.");
        return;
    }

    // Clear any previous error messages
    ui->lbl_mk_trnsf_err_2->clear();

    // Request ID for Make Transfer
    quint8 requestId = 7; // You can choose any unused ID

    // Create a JSON object for the transfer request
    QJsonObject transferRequest;
    transferRequest["requestId"] = static_cast<int>(requestId);
    transferRequest["fromAccountNumber"] = accountNumber;
    transferRequest["toAccountNumber"] = toAccountNumber;
    transferRequest["amount"] = amount;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(transferRequest);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
    socket->flush();
}

void client::handleMakeTransferResponse(const QJsonObject &responseObject)
{
    ui->lbl_mk_trnsf_err_2->clear();
    double newFromBalance = responseObject["newFromBalance"].toDouble();
    double newToBalance = responseObject["newToBalance"].toDouble();
    bool transferSuccess = responseObject["transferSuccess"].toBool();

    if (transferSuccess)
    {
        ui->lbl_mk_trnsf_err_2->setText("Transfer successful :)");
        qDebug() << "Transfer successful. New 'From' balance: $" << newFromBalance
                 << ". New 'To' balance: $" << newToBalance;
    }
    else
    {
        QString errorMessage = responseObject["errorMessage"].toString();
        ui->lbl_mk_trnsf_err_2->setText("Transfer failed: " + errorMessage);
        qDebug() << "Transfer failed: " << errorMessage;
    }
}


void client::on_pbn_view_transaction_histroy_2_clicked()
{
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

void client::handleViewTransactionHistoryResponse(const QJsonObject &responseObject)
{
    ui->lbl_err_transaction_history_3->clear();

    bool viewTransactionHistorySuccess = responseObject["viewTransactionHistorySuccess"].toBool();

    if (viewTransactionHistorySuccess)
    {
        // Clear existing data in tbl_transactionhistory
        ui->tbl_view_histroy_transaction_2->clearContents();
        ui->tbl_view_histroy_transaction_2->setRowCount(0);

        // Get the transaction history array from the response
        QJsonArray transactionHistoryArray = responseObject["transactionHistory"].toArray();



        // Populate tbl_transactionhistory with transaction history data
        int row = 0;
        for (const auto &transactionDataValue : transactionHistoryArray)
        {
            QJsonObject transactionData = transactionDataValue.toObject();

            ui->tbl_view_histroy_transaction_2->insertRow(row);
            ui->tbl_view_histroy_transaction_2->setItem(row, 0, new QTableWidgetItem(QString::number(transactionData["TransactionID"].toVariant().toLongLong())));
            ui->tbl_view_histroy_transaction_2->setItem(row, 1, new QTableWidgetItem(QString::number(transactionData["Amount"].toDouble())));
            ui->tbl_view_histroy_transaction_2->setItem(row, 2, new QTableWidgetItem(transactionData["Date"].toString()));
            ui->tbl_view_histroy_transaction_2->setItem(row, 3, new QTableWidgetItem(transactionData["Time"].toString()));
            row++;
        }

        qDebug() << "View Transaction History successful.";
    }
    else
    {
        ui->lbl_err_transaction_history_3->setText("Failed to view transaction history.");
        qDebug() << "Failed to view Transaction History.";
    }
}


void client::on_pushButton_get_account_number_2_clicked()
{
    ui->label_error_2->clear();
    // Request ID for Get Account Number
    quint8 requestId = 1;
    // Get the username from the UI
    QString username = ui->lineEdit_username_2->text();
    // Check if Username and Password fields are not empty
    if (username.isEmpty())
    {
        qDebug() << "Warning: Please fill in the username and password.";
        ui->label_error_2->setText("Can't be empty.");
        return;
    }

    // Check if the Username field contains only alphanumeric characters and underscores
    if (!username.contains(usernameRegex))
    {
        qDebug() << "Warning: Username must only contain alphanumeric characters.";
        ui->label_error_2->setText("Can't have any special chars only Underscores");
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

void client::handleGetAccountNumberResponse(const QJsonObject &responseObject)
{
    ui->label_error_2->clear();
    ui->label_view_account_number_2->setText("Account Number: ");
    accountNumber = responseObject["accountNumber"].toVariant().toLongLong();
    bool userFound = responseObject["userFound"].toBool();
    if (userFound)
    {
        ui->label_view_account_number_2->setText("Number: "
                                               + QString::number(accountNumber));
        ui->label_error_2->setText("Success :)");
        qDebug() << "User found. Account Number: " << accountNumber;
    }
    else
    {
        ui->label_error_2->setText("User Not Found!");
        qDebug() << "User not found.";
    }
}


void client::on_pbn_view_balance_2_clicked()
{
    ui->label_error_viewbalance_2->clear();
    // Request ID for View Account Balance
    quint8 requestId = 10;
    // Get the account number from the UI
    qint64 accountNumber = ui->lnedit_accountnumber_2->text().toLongLong();

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);
    requestObject["accountNumber"] = accountNumber;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
}

void client::adminHandleViewAccountBalanceResponse(const QJsonObject &responseObject)
{
    qDebug() <<responseObject;
    ui->label_error_viewbalance_2->clear();
    ui->lbl_viewbalance_2->setText("Balance: ");
    double accountBalance = responseObject["balance"].toDouble();
    bool viewBalanceSuccess = responseObject["accountFound"].toBool();

    if (viewBalanceSuccess)
    {
        ui->lbl_viewbalance_2->setText("Balance: $" + QString::number(accountBalance));
        ui->label_error_viewbalance_2->setText("Success :)");
        qDebug() << "View Account Balance successful. Balance: $" << accountBalance;
    }
    else
    {
        ui->label_error_viewbalance_2->setText("Failed to view Account Balance!");
        qDebug() << "Failed to view Account Balance.";
    }
}


void client::on_pbn_create_new_account_2_clicked()
{
    ui->lbl_error_create_2->clear();

    // Request ID for Create Account
    quint8 requestId = 3;

    // Get the necessary information from the UI
    QString username = ui->lnedit_username_input_2->text();
    QString password = ui->lnedit_password_input_2->text();
    QString name = ui->lnedit_name_input_2->text();
    quint8 age = ui->lnedit_age_input_2->text().toUInt();
    bool isAdmin = ui->chkbox_admin_2->isChecked();

    // Check if any of the required fields are empty
    if (username.isEmpty() ||
        password.isEmpty() ||
        name.isEmpty())
    {
        qDebug() << "Warning: Please fill in all the required fields.";
        ui->lbl_error_create_2->setText("All fields are required.");
        return;
    }

    // Check if the Username field contains only alphanumeric characters and underscores
    if (!username.contains(usernameRegex))
    {
        qDebug() << "Warning: Username must only contain alphanumeric characters.";
        ui->lbl_error_create_2->setText("Username can only have alphanumeric characters.");
        return;
    }

    // Check if the Password field contains whitespace
    if (password.contains(passwordRegex))
    {
        qDebug() << "Warning: Password must not contain whitespace.";
        ui->lbl_error_create_2->setText("Password must not contain whitespace.");
        return;
    }

    // Check if the age is between 18 and 120
    if (age < 18 || age > 120)
    {
        qDebug() << "Warning: Age must be between 18 and 120.";
        ui->lbl_error_create_2->setText("Age must be between 18 and 120.");
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

void client::handleCreateNewAccountResponse(const QJsonObject &responseObject)
{
    qDebug() << responseObject;
    ui->lbl_error_create_2->clear();

    bool createAccountSuccess = responseObject["createAccountSuccess"].toBool();
    qint64 accountNumber = responseObject["accountNumber"].toVariant().toLongLong();

    if (createAccountSuccess)
    {
        ui->lbl_error_create_2->setText("Account created. Account Number: "
                                      + QString::number(accountNumber));
        qDebug() << "Create Account successful. Account Number: " << accountNumber;
    }
    else
    {
        QString errorMessage = responseObject["errorMessage"].toString();
        ui->lbl_error_create_2->setText("Failed Error: " + errorMessage);
        qDebug() << "Failed to create account. Error: " << errorMessage;
    }
}




void client::on_pbn_view_database_2_clicked()
{
    ui->lbl_view_database_error_2->clear();

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

void client::handleFetchAllUserDataResponse(const QJsonObject &responseObject)
{
    ui->label_error_2->clear();

    bool fetchUserDataSuccess = responseObject["fetchUserDataSuccess"].toBool();

    if (fetchUserDataSuccess)
    {
        // Clear existing data in tbl_view_database
        ui->tbl_view_database_2->clearContents();
        ui->tbl_view_database_2->setRowCount(0);

        // Get the user data array from the response
        QJsonArray userDataArray = responseObject["userData"].toArray();

        // Populate tbl_view_database with user data
        int row = 0;
        for (const auto &userDataValue : userDataArray)
        {
            QJsonObject userData = userDataValue.toObject();

            ui->tbl_view_database_2->insertRow(row);
            ui->tbl_view_database_2->setItem(row, 0, new QTableWidgetItem(QString::number(userData["AccountNumber"].toInt())));
            ui->tbl_view_database_2->setItem(row, 1, new QTableWidgetItem(userData["Username"].toString()));
            ui->tbl_view_database_2->setItem(row, 2, new QTableWidgetItem(userData["Name"].toString()));
            ui->tbl_view_database_2->setItem(row, 3, new QTableWidgetItem(QString::number(userData["Balance"].toDouble())));
            ui->tbl_view_database_2->setItem(row, 4, new QTableWidgetItem(QString::number(userData["Age"].toInt())));

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
        ui->label_error_2->setText("Failed to fetch user data.");
        qDebug() << "Failed to fetch user data.";
    }
}

void client::on_pbn_view_transaction_history_2_clicked()
{

    // Get the account number to view transaction history
    qint64 accountNumber=ui->lnedit_act_number_transaction_history_2->text().toInt();

    // Validate the accountNumber
    if (accountNumber <= 0) {
        // Display an error message
        ui->lbl_err_transaction_history_2->setText("Invalid Account Number. Please enter a valid number.");
        return;
    }

    // Clear any previous error messages
    ui->lbl_err_transaction_history_2->clear();

    // Request ID for View Transaction History
    quint8 requestId = 11;

    // Construct the request JSON object
    QJsonObject requestObject;
    requestObject["requestId"] = static_cast<int>(requestId);
    requestObject["accountNumber"] = accountNumber;

    // Convert the JSON object to a JSON document
    QJsonDocument jsonRequest(requestObject);

    // Send the request to the server
    socket->write(jsonRequest.toJson());
}

void client::adminHandleViewTransactionHistoryResponse(const QJsonObject &responseObject)
{
    ui->lbl_err_transaction_history_2->clear();

    bool viewTransactionHistorySuccess = responseObject["viewTransactionHistorySuccess"].toBool();

    if (viewTransactionHistorySuccess)
    {
        // Clear existing data in tbl_transaction_history
        ui->tbl_transaction_history_2->clearContents();
        ui->tbl_transaction_history_2->setRowCount(0);

        // Get the transaction history array from the response
        QJsonArray transactionHistoryArray = responseObject["transactionHistory"].toArray();


        // Populate tbl_transaction_history with transaction history data
        int row = 0;
        for (const auto &transactionDataValue : transactionHistoryArray)
        {
            QJsonObject transactionData = transactionDataValue.toObject();

            ui->tbl_transaction_history_2->insertRow(row);
            ui->tbl_transaction_history_2->setItem(row, 0, new QTableWidgetItem(QString::number(transactionData["TransactionID"].toVariant().toLongLong())));
            ui->tbl_transaction_history_2->setItem(row, 1, new QTableWidgetItem(QString::number(transactionData["Amount"].toDouble())));
            ui->tbl_transaction_history_2->setItem(row, 2, new QTableWidgetItem(transactionData["Date"].toString()));
            ui->tbl_transaction_history_2->setItem(row, 3, new QTableWidgetItem(transactionData["Time"].toString()));
            row++;
        }

        qDebug() << "View Transaction History successful.";
    }
    else
    {
        ui->lbl_err_transaction_history_2->setText("Failed to view transaction history.");
        qDebug() << "Failed to view Transaction History.";
    }
}


void client::on_chkbox_admin_2_clicked()
{
    if(ui->chkbox_admin_2->isChecked())
    {
        ui->lbl_error_create_2->setText("Admin");
    }
    else
    {
        ui->lbl_error_create_2->setText("Client");
    }
}

