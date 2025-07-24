#pragma once
#include "pch.h"
#include "logic.h"

class Logic;
class ClientSession;
class WaitingManager;

using RecvPakcetType = std::function<void(ClientSession*, const char*, uint16_t)>;

class LogicServer : public Core, public Singleton<LogicServer>
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
	ClientSession* GetSession(int sessionId){return Core::GetSession(sessionId);}
	bool Init(int maxSessionCount, int maxWaitingCount);
	bool HasFreeSlot();
	void BindSession(ClientSession* session);

private:
	std::unordered_map<size_t, RecvPakcetType> mRecvFuncMap;
	std::atomic<bool> mIsRunning;
	std::unordered_map<int, ClientSession*> mActiveSessionMap;
	std::mutex mActiveSessionLock;
	std::unique_ptr<ThreadManager> mThread;
	int mMaxSession{ 0 };
};
