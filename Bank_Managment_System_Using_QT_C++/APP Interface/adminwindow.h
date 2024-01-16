#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include "client.h"

namespace Ui
{
class AdminWindow;
}

class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr,
                         qint64 accountNumber = 0,
                         QTcpSocket *socket = nullptr);
    ~AdminWindow();

signals:
    void finished();

private slots:
    void readyRead();

    void on_pushButton_get_account_number_clicked();

    void on_pbn_view_balance_clicked();

    void on_pbn_create_new_account_clicked();

    void on_pbn_delete_account_clicked();

    void on_pbn_view_database_clicked();

    void on_pbn_view_transaction_history_clicked();

    void on_pbn_update_account_clicked();

private:
    Ui::AdminWindow *ui;
    QTcpSocket *socket;
    qint64 accountNumber;
    QByteArray responseData;

    // Regular expressions for username and password validation
    static const QRegularExpression usernameRegex;
    static const QRegularExpression passwordRegex;

    void handleCreateNewAccountResponse(const QJsonObject &responseObject);
    void handleGetAccountNumberResponse(const QJsonObject &responseObject);
    void handleViewAccountBalanceResponse(const QJsonObject &responseObject);
    void handleDeleteAccountResponse(const QJsonObject &responseObject);
    void handleFetchAllUserDataResponse(const QJsonObject &responseObject);
    void handleViewTransactionHistoryResponse(const QJsonObject &responseObject);
    void handleUpdateAccountResponse(const QJsonObject &responseObject);
};

#endif // ADMINWINDOW_H
