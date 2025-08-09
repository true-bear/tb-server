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
    [[nodiscard]]LogicManager();
    ~LogicManager();

    bool Init(SessionGetFunc getSession, const int threadCount = 1);
    void Start();
    void Stop();

    [[nodiscard]] void DisPatchPacket(int sessionId, std::span<const std::byte> data);
    int ShardIndex(int sessionId) const noexcept;
private:

    struct Shard 
    {
        std::unique_ptr<LogicThread> thread;
        std::unique_ptr<boost::lockfree::queue<PacketEx*>> queue;
    };

    std::vector<Shard> mShards;

    LogicDispatch mDispatcher;
    SessionGetFunc mGetSessionFunc;
};

