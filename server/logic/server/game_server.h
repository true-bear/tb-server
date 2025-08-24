#pragma once
#include "../logic/logic.h"

import core.engine;
import util.singleton;
import iocp.session;

class GameServer : public Core, public Singleton<GameServer>
{
public:
    GameServer();
    virtual ~GameServer();

    bool Init();

    virtual void Run();
    virtual void Stop();

private:
    int mMaxSession = 0;
    std::atomic<std::uint64_t> mGatewaySid{ 0 };
    LogicManager mLogicManager;
};