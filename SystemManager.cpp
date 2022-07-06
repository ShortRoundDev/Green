#include "SystemManager.h"

#include "GraphicsManager.h"
#include "GTypes.h"
#include "Logger.h"

#include "Windows.h"

#include <algorithm>

static Logger logger = CreateLogger("SystemManager");

////////// PUBLIC //////////

bool SystemManager::start()
{
    if (!loadCmdLine())
    {
        return false;
    }

    if (!m_configFilePath.empty() && !loadCfg(m_configFilePath))
    {
        return false;
    }

    if (!parseParameters())
    {
        return false;
    }


    return true;
}

bool SystemManager::shutDown()
{
    return true;
}


const SystemVars& SystemManager::getVars()
{
    return m_systemVars;
}

bool SystemManager::isRunning()
{
    return m_running;
}

void SystemManager::setRunning(bool running)
{
    m_running = running;
}

bool SystemManager::readFile(std::string fileName, u8** buffer, sz maxSize, sz* outSize)
{
    return readFile(fileName.c_str(), buffer, maxSize, outSize);
}

bool SystemManager::readFile(const char* fileName, u8** buffer, sz maxSize, sz* outSize)
{
    HANDLE handle = CreateFileA(
        fileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (handle == INVALID_HANDLE_VALUE)
    {
        logger.err("Failed to open file %s! Got 0x%x", fileName, GetLastError());
        return false;
    }

    return readFileHandle(handle, buffer, maxSize, outSize);
}

bool SystemManager::readFile(std::wstring fileName, u8** buffer, sz maxSize, sz* outSize)
{
    return readFile(fileName.c_str(), buffer, maxSize, outSize);
}

bool SystemManager::readFile(const wchar_t* fileName, u8** buffer, sz maxSize, sz* outSize)
{
    HANDLE handle = CreateFileW(
        fileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE)
    {
        logger.err(L"Failed to open file %s! Got 0x%x", fileName, GetLastError());
        return false;
    }

    return readFileHandle(handle, buffer, maxSize, outSize);
}

////////// PRIVATE //////////

bool SystemManager::readFileHandle(HANDLE file, u8** buffer, sz maxSize, sz* outSize)
{

    DWORD size = GetFileSize(file, NULL);
    if (maxSize != 0 && size > maxSize)
    {
        logger.warn("WARNING: File larger than buffer! Truncating...");
        size = (u32)maxSize;
    }

    *outSize = size;

    *buffer = (u8*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size + 1);

    return ::ReadFile(file, *buffer, size, NULL, NULL);
}

bool SystemManager::loadCmdLine()
{
    i32 numArgs = 0;
    LPWSTR* args = CommandLineToArgvW(
        GetCommandLine(),
        &numArgs
    );
    std::wstring key;
    for (int i = 1; i < numArgs; i++)
    {
        if (args[i][0] == '-')
        {
            if (key.empty())
            {
                key = std::wstring(args[i] + 1);
            }
            else
            {
                m_args[key] = L"true";
                key = std::wstring(args[i] + 1);
            }
        }
        else
        {
            if (key.empty())
            {
                logger.err(L"Error: Value without key! Skipping %s", args[i]);
                continue;
            }
            std::transform(
                key.begin(),
                key.end(),
                key.begin(),
                tolower
            );

            m_args[key] = std::wstring(args[i]);
            key.clear();
        }
    }

    auto cfg = m_args.find(L"cfg");
    if (cfg != m_args.end())
    {
        m_configFilePath = cfg->second;
    }

    return true;
}

bool SystemManager::loadCfg(std::wstring filename)
{
    static char cfgBuffer[4096];
    wchar_t wBuffer[128];
    ZeroMemory(cfgBuffer, 4096);

    HANDLE cfgFile = CreateFile(
        filename.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (cfgFile == INVALID_HANDLE_VALUE)
    {
        logger.err(L"Failed to open cfg file %s! Got 0x%x", filename.c_str(), GetLastError());
        return false;
    }
    DWORD size = GetFileSize(cfgFile, NULL);
    if (size > 4096)
    {
        size = 4096;
    }

    if (!ReadFile(cfgFile, cfgBuffer, size, NULL, NULL))
    {
        logger.err(L"Failed to read cfg file %s! Got %x", filename.c_str(), GetLastError());
        return false;
    }

    char* start = cfgBuffer;

    std::wstring key, value;
    char* c = cfgBuffer;
    for (;*c != '\0';)
    {
        if (*c == '\n')
        {
            if (start - c == 0) // skip blank lines
            {
                goto Next;
            }

            sz size;
            mbstowcs_s(&size, wBuffer, start, (c + 1) - start); // convert line to utf-16

            if (!parseCfgLine(wBuffer, key, value)) // parse individual line
            {
                logger.err(L"Failed to parse cfg line! Got key-less value %s!", wBuffer);
                goto Next;
            }

            //keys are case-insensitive
            this->toLower(key);

            m_args[key] = value;
        Next:
            start = ++c; // set start to new line start
        }
        else
        {
            c++;
        }
    }

    if (*start != L'\0') // still one line left
    {
        sz size;
        mbstowcs_s(&size, wBuffer, start, (c + 1) - start);
        if (!parseCfgLine(wBuffer, key, value))
        {
            logger.err(L"Failed to parse cfg line! Got key-less value %s!", wBuffer);
        }
        else
        {
            this->toLower(key);

            m_args[key] = value;
        }
    }

    return true;
}

bool SystemManager::parseCfgLine(wchar_t* wBuffer, std::wstring& key, std::wstring& value)
{
    wchar_t* wKey = wBuffer;
    wchar_t* wKeyEnd = wBuffer;
    bool encounteredAlpha = false;
    wchar_t* i = wBuffer;
    // Search for key
    for (; *i != L'\n' && *i != L'\0'; i++)
    {
        //trim leading whitespace
        if (!encounteredAlpha && (*i == L' ' || *i == L'\t'))
        {
            wKey++;
        }
        else if (!encounteredAlpha)
        {
            encounteredAlpha = true;
        }
        else if (encounteredAlpha && !(*i == L' ' || *i == L'\t' || *i == L'=')) // trim trailing whitespace
        {
            wKeyEnd = i;
        }
        
        if (*i == L'=') // early exit on =
        {
            break; //Key Done
        }
    }

    if (wKeyEnd <= wKey) // empty key or key is just whitespace
    {
        return false;
    }

    if (*i == L'\n') // key has no = sign, just a boolean flag
    {
        *(wKeyEnd + 1) = L'\0';
        key = std::wstring(wKey);
        value = L"true";
        return true;
    }

    *(wKeyEnd + 1) = L'\0'; // null terminate the key

    key = std::wstring(wKey);
    
    wchar_t* wValue = ++i; // move past = sign
    wchar_t* wValueEnd = wValue;
    encounteredAlpha = false;
    for (; *i != L'\n' && *i != L'\0'; i++)
    {
        //trim leading whitespace
        if (!encounteredAlpha && (*i == L' ' || *i == L'\t'))
        {
            wValue++;
        }
        else if (!encounteredAlpha)
        {
            encounteredAlpha = true;
        }
        else if (encounteredAlpha && !(*i == L' ' || *i == L'\t')) // trim trailing whitespace
        {
            wValueEnd = i;
        }
        // no early exit, just go until end of line or end of file
    }

    if (wValueEnd <= wValue) // key is empty or whitespace
    {
        value = L"true";
    }
    else
    {
        *(wValueEnd + 1) = L'\0'; // null terminate key
        value = std::wstring(wValue);
    }
    return true;
}

inline void SystemManager::toLower(std::wstring& str)
{
    std::transform(
        str.begin(),
        str.end(),
        str.begin(),
        ::tolower
    );
}

bool SystemManager::parseParameters()
{
    auto width = m_args.find(L"width");
    if (width != m_args.end())
    {
        m_systemVars.width = _wtoi(width->second.c_str());
    }

    auto height = m_args.find(L"height");
    if (height != m_args.end())
    {
        m_systemVars.height = _wtoi(height->second.c_str());
    }

    auto fullScreen = m_args.find(L"fullscreen");
    if (fullScreen != m_args.end())
    {
        m_systemVars.fullScreen = fullScreen->second == L"true";
    }

    auto title = m_args.find(L"title");
    if (title != m_args.end())
    {
        m_systemVars.title = title->second;
    }

    auto debugMode = m_args.find(L"debug");
    if (debugMode != m_args.end())
    {
        m_systemVars.debugMode = debugMode->second == L"true";
    }

    return true;
}