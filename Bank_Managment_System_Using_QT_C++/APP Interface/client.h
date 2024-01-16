#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QRegularExpression>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>

namespace Ui
{
    class client;
}

class client : public QMainWindow
{
    Q_OBJECT

public:
    client(QWidget *parent = nullptr);
    ~client();

public slots:
    void readyRead();

private slots:
    void on_pushButton_login_clicked();
    void on_pushButton_logout_clicked();
    void on_pushButton_get_account_number_4_clicked();
    void on_pbn_view_balance_4_clicked();

    void on_pbn_make_trasnaction_2_clicked();

    void on_pbn_mk_transfer_2_clicked();

    void on_pbn_view_transaction_histroy_2_clicked();

    void on_pushButton_get_account_number_2_clicked();

    void on_pbn_view_balance_2_clicked();

    void on_pbn_create_new_account_2_clicked();

    void on_pbn_view_database_2_clicked();

    void on_pbn_view_transaction_history_2_clicked();


    void on_chkbox_admin_2_clicked();

private:
    Ui::client *ui;
    QTcpSocket *socket;
    qint64 accountNumber;

    static const QRegularExpression usernameRegex;
    static const QRegularExpression passwordRegex;

    void handleLoginResponse(const QJsonObject &responseObject);
    void handleViewAccountBalanceResponse(const QJsonObject &responseObject);
    void handleMakeTransactionResponse(const QJsonObject &responseObject);
    void handleMakeTransferResponse(const QJsonObject &responseObject);
    void handleViewTransactionHistoryResponse(const QJsonObject &responseObject);
    void handleGetAccountNumberResponse(const QJsonObject &responseObject);
    void adminHandleViewAccountBalanceResponse(const QJsonObject &responseObject);
    void handleCreateNewAccountResponse(const QJsonObject &responseObject);
    void handleDeleteAccountResponse(const QJsonObject &responseObject);
    void handleFetchAllUserDataResponse(const QJsonObject &responseObject);
    void adminHandleViewTransactionHistoryResponse(const QJsonObject &responseObject);
    void handleUpdateAccountResponse(const QJsonObject &responseObject);
};

#endif // CLIENT_H
