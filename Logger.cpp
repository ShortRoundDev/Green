#include "Logger.h"

#include "LogManager.h"

#include "Windows.h"

#include <stdarg.h>
#include <wchar.h>

thread_local char Logger::s_buffer[LOG_BUFFER_SIZE];

Logger CreateLogger(const char* fileName)
{
    return Logger(fileName);
}

Logger::Logger(const char* filePrefix)
{
    this->m_filePrefix = filePrefix;
    this->m_filePrefixLength = (u32)strnlen(filePrefix, 64);

    size_t size;
    mbstowcs_s(&size, (wchar_t*)m_filePrefixW, 64, m_filePrefix, 64);

    auto Logs = LogManager::getLogManager();

    this->m_infoStream = Logs->getInfoStream();
    this->m_warnStream = Logs->getWarnStream();
    this->m_errStream = Logs->getErrStream();
}

Logger::~Logger()
{

}

void Logger::info(const char* fmt...)
{
    va_list args;
    va_start(args, fmt);

    write(m_infoStream, fmt, args);
}

void Logger::info(const wchar_t* fmt...)
{
    va_list args;
    va_start(args, fmt);

    write(m_infoStream, fmt, args);
}

void Logger::warn(const char* fmt...)
{
    va_list args;
    va_start(args, fmt);

    write(m_warnStream, fmt, args);
}

void Logger::warn(const wchar_t* fmt...)
{
    va_list args;
    va_start(args, fmt);

    write(m_warnStream, fmt, args);
}

void Logger::err(const char* fmt...)
{
    va_list args;
    va_start(args, fmt);
    write(m_errStream, fmt, args);
}

void Logger::err(const wchar_t* fmt...)
{
    va_list args;
    va_start(args, fmt);
    write(m_errStream, fmt, args);
}

void Logger::write(FILE* stream, const char* fmt, va_list args)
{
    u32 iter = 1 + m_filePrefixLength;
    s_buffer[0] = '[';
    memcpy(s_buffer + 1, m_filePrefix, m_filePrefixLength);
    s_buffer[iter++] = ']';
    s_buffer[iter++] = ' ';

    u32 fmtLength = (u32)strnlen(fmt, LOG_BUFFER_SIZE - (iter + 2));
    memcpy(s_buffer + iter, fmt, fmtLength);
    iter += fmtLength;
    s_buffer[iter++] = '\n';
    s_buffer[iter++] = 0;
    vfprintf(stream, s_buffer, args);
}

void Logger::write(FILE* stream, const wchar_t* fmt, va_list args)
{
    thread_local static wchar_t* wBuff = (wchar_t*)s_buffer;

    u32 iter = 1 + m_filePrefixLength;
    wBuff[0] = L'[';
    memcpy(wBuff + 1, m_filePrefixW, m_filePrefixLength * sizeof(wchar_t));
    wBuff[iter++] = L']';
    wBuff[iter++] = ' ';

    u32 fmtLength = (u32)wcsnlen(fmt, W_LOG_BUFFER_SIZE - (iter + 2));
    memcpy(wBuff + iter, fmt, fmtLength * sizeof(wchar_t));
    iter += fmtLength;
    wBuff[iter++] = L'\n';
    wBuff[iter++] = L'\0';
    vfwprintf(stream, wBuff, args);
}