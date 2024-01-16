#include "Logger.h"

Logger::Logger(const QString &tag, QObject *parent)
    : QObject(parent), logTag(tag)
{}

Logger::~Logger()
{}

void Logger::log(const QString &message)
{
    QMutexLocker locker(&mutex);

    QString logFilePath = "common_log.txt";

    logFile.setFileName(logFilePath);

    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        textStream.setDevice(&logFile);
    }
    else
    {
        qDebug() << "Failed to open log file:" << logFilePath;
    }
    QString formattedMessage = QString("%1: %2").arg(logTag, message);
    textStream << formattedMessage << '\n';
    textStream.flush();
    logFile.close();

    qDebug() << formattedMessage;
}
