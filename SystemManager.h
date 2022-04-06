#pragma once

#include "IManager.h"

#include "GTypes.h"
#include "SystemVars.h"

#include "Windows.h"

#include <unordered_map>
#include <string>

class SystemManager : public IManager
{
public:
    virtual bool start();
    virtual bool shutDown();

    const SystemVars& getVars();
    bool isRunning();
    void setRunning(bool running);

    bool readFile(std::string fileName, u8** buffer, size_t maxSize, size_t* outSize);
    bool readFile(const char* fileName, u8** buffer, size_t maxSize, size_t* outSize);
    bool readFile(std::wstring fileName, u8** buffer, size_t maxSize, size_t* outSize);
    bool readFile(const wchar_t* fileName, u8** buffer, size_t maxSize, size_t* outSize);
private:
    std::wstring    m_configFilePath = L"Default.cfg";
    SystemVars      m_systemVars;
    bool            m_running;
    std::unordered_map<std::wstring, std::wstring> m_args;

    bool loadCmdLine();
    bool loadCfg(std::wstring filename);
    bool parseCfgLine(wchar_t* wBuffer, std::wstring& key, std::wstring& value);
    bool parseParameters();
    bool readFileHandle(HANDLE file, u8** buffer, size_t maxSize, size_t* outSize);



    // INPUT

    inline void toLower(std::wstring& str); // helper string func. Should probably go in some util class somewhere
};

extern SystemManager System;