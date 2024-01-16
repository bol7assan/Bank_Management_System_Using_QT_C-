#ifndef USERWINDOW_H
#define USERWINDOW_H

#include "client.h"

namespace Ui
{
class UserWindow;
}

class UserWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UserWindow(QWidget *parent = nullptr,
                        qint64 accountNumber = 0,
                        QTcpSocket *socket = nullptr);
    ~UserWindow();

signals:
    void finished();

private slots:
    void readyRead();

    void on_pushButton_get_account_number_clicked();

    void on_pbn_view_balance_clicked();

    void on_pbn_make_trasnaction_clicked();

    void on_pbn_mk_transfer_clicked();

    void on_pbn_view_transaction_histroy_clicked();

private:
    Ui::UserWindow *ui;
    QTcpSocket *socket;
    qint64 accountNumber;
    QByteArray responseData;

    void handleViewAccountBalanceResponse(const QJsonObject &responseObject);
    void handleMakeTransactionResponse(const QJsonObject &responseObject);
    void handleMakeTransferResponse(const QJsonObject &responseObject);
    void handleViewTransactionHistoryResponse(const QJsonObject &responseObject);
};

#endif // USERWINDOW_H
