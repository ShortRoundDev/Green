#include "LogManager.h"

#include <Windows.h>

#include "SystemManager.h"

bool LogManager::start()
{
    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    m_initialized = true;
    return true;
}

bool LogManager::shutDown()
{
    return true;
}

FILE* LogManager::getInfoStream()
{
    return m_infoStream;
}

void LogManager::setInfoStream(FILE* stream)
{
    m_infoStream = stream;
}

FILE* LogManager::getWarnStream()
{
    return m_warnStream;
}

void LogManager::setWarnStream(FILE* stream)
{
    m_warnStream = stream;
}

FILE* LogManager::getErrStream()
{
    return m_errStream;
}

void LogManager::setErrStream(FILE* stream)
{
    m_errStream = stream;
}

LogManager* LogManager::getLogManager()
{
    static LogManager logs;
    if (!logs.isInitialized())
    {
        logs.start();
    }
    return &logs;
}

bool LogManager::isInitialized()
{
    return m_initialized;
}