#pragma once
class IManager
{
public:
    virtual bool start() = 0;
    virtual bool shutDown() = 0;
};