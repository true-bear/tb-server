#pragma once
#include "logic.h"

import core.engine;
import util.singleton;
import iocp.session;

using RecvPakcetType = std::function<void(Session*, const char*, uint16_t)>;

class LogicServer : public Core, public Singleton<LogicServer>
{
public:
    LogicServer();
    virtual ~LogicServer();

    bool Init(int maxSessionCount);
    virtual void Run();
    virtual void Stop();

private:
    int mMaxSession = 0;

    LogicManager mLogicManager;
};