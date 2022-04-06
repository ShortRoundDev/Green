#pragma once

#include "IManager.h"

#include <iostream>

class LogManager : public IManager
{
public:
    virtual bool start();
    virtual bool shutDown();

    FILE* getInfoStream();
    void setInfoStream(FILE* stream);

    FILE* getWarnStream();
    void setWarnStream(FILE* stream);

    FILE* getErrStream();
    void setErrStream(FILE* stream);

    static LogManager* getLogManager();

    bool isInitialized();

private:
    bool m_initialized = false;

    FILE* m_infoStream = stdout;
    FILE* m_warnStream = stdout;
    FILE* m_errStream = stderr;
};
