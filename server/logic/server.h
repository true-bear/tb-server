#pragma once
#include "pch.h"
#include "logic.h"

class Logic;
class IocpSession;
class WaitingManager;

using RecvPakcetType = std::function<void(IocpSession*, const char*, uint16_t)>;

class LogicServer : public IocpCore, public Singleton<LogicServer>
{
public:
	LogicServer();
	virtual ~LogicServer();

public:
	virtual void Run();
	virtual void Stop();
	virtual void OnRecv(unsigned int uID, unsigned long ioSize);
	virtual void OnAccept(unsigned int uID, unsigned long long completekey);
	virtual bool OnClose(unsigned int uID);
	virtual void OnSend(unsigned int uID, unsigned long ioSize);

public:
	IocpSession* GetSession(int sessionId){return IocpCore::GetSession(sessionId);}
	bool Init(int maxSessionCount, int maxWaitingCount);
	bool HasFreeSlot();
	void BindSession(IocpSession* session);

private:
	std::unordered_map<size_t, RecvPakcetType> mRecvFuncMap;
	std::atomic<bool> mIsRunning;
	std::unordered_map<int, IocpSession*> mActiveSessionMap;
	std::mutex mActiveSessionLock;
	std::unique_ptr<ThreadManager> mThread;
	int mMaxSession{ 0 };
};
