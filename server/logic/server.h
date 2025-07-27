#pragma once
#include "pch.h"
#include "logic.h"

class Logic;
class Session;

using RecvPakcetType = std::function<void(Session*, const char*, uint16_t)>;

class LogicServer : public Core, public Singleton<LogicServer>
{
public:
    LogicServer();
    virtual ~LogicServer();

    bool Init(int maxSessionCount);
    virtual void Run();
    virtual void Stop();

    virtual void OnRecv(unsigned int uID, unsigned long ioSize);
    virtual void OnAccept(unsigned int uID, unsigned long long completekey);
    virtual bool OnClose(unsigned int uID);
    virtual void OnSend(unsigned int uID, unsigned long ioSize);

    Session* GetSession(int sessionId) { return Core::GetSession(sessionId); }
    bool HasFreeSlot();
    void BindSession(Session* session);

private:
    int mMaxSession = 0;
    std::unordered_map<int, Session*> mActiveSessionMap;
    std::mutex mActiveSessionLock;

    LogicManager mLogicManager;
};