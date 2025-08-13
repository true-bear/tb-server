#pragma once
#include "../logic/logic.h"

import core.engine;
import util.singleton;
import iocp.session;

using RecvPakcetType = std::function<void(Session*, const char*, uint16_t)>;

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

    LogicManager mLogicManager;
};