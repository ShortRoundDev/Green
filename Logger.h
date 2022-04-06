#pragma once
#include "GTypes.h"

#include <iostream>
#include <tuple>

constexpr u64 LOG_BUFFER_SIZE = 4096;
constexpr u64 W_LOG_BUFFER_SIZE = LOG_BUFFER_SIZE / sizeof(wchar_t);

class Logger
{
public:
    Logger(const char* filePrefix);
    ~Logger();

    void info(const char* fmt...);
    void info(const wchar_t* fmt...);

    void warn(const char* fmt...);
    void warn(const wchar_t* fmt...);

    void err(const char* fmt...);
    void err(const wchar_t* fmt...);

private:
    void write(FILE* stream, const char* fmt, va_list args);
    void write(FILE* stream, const wchar_t* fmt, va_list args);

    const char* m_filePrefix;
    wchar_t m_filePrefixW[64];
    u32 m_filePrefixLength;

    thread_local static char s_buffer[LOG_BUFFER_SIZE];

    FILE* m_infoStream;
    FILE* m_warnStream;
    FILE* m_errStream;
};

Logger CreateLogger(const char* fileName);
