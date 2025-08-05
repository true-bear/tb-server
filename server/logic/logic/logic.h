#pragma once

#include "../thread/logicThread.h"

import util.singleton;
import iocp.session;

class LogicDispatch;
class PacketEx;
class Session;
class LogicDispatch;

using SessionGetFunc = std::function<Session* (int)>;

class LogicManager : public Singleton<LogicManager>
{
public:
    LogicManager();
    ~LogicManager();

    bool Init(SessionGetFunc getSession);
    void Start();
    void Stop();

    void DisPatchPacket(int sessionId, std::span<const std::byte> data);

private:
    std::unique_ptr<LogicThread> mLogicThread;
    LogicDispatch mDispatcher;
    static boost::lockfree::queue<PacketEx*> mPacketQueue;
    SessionGetFunc mGetSessionFunc;
};

