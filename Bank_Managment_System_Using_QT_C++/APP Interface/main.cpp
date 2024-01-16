#include <QApplication>
#include <QCoreApplication>
#include "client.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    client BankApp;
    BankApp.show();
    return a.exec();
}
