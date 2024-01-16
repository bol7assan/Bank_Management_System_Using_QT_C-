#include <QCoreApplication>
#include <signal.h>
#include "databasemanager.h"
#include "Server.h"
#include "Logger.h"

void handleSignal(int signal);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    signal(SIGINT, handleSignal);

    // Initialize the database
    DatabaseManager databaseManager("InitializeDatabase",&a);
    databaseManager.initializeDatabase();

    // Create the Server object
    Server server(&a);

    Logger mainLogger("Main");

    if (!server.isListening())
    {
        mainLogger.log("Failed to start the server.");
        return 1;
    }

    mainLogger.log("Event loop Started.");

    a.processEvents();
    return a.exec();
}

void handleSignal(int signal)
{
    Q_UNUSED(signal);
    Logger signalLogger("ExitSignal");
    signalLogger.log("Received exit signal. Initiating server shutdown.");
    QCoreApplication::quit();
}
