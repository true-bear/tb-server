#include "pch.h"
#include "server.h"
#include "logic/logic.h"

import util.singleton;
import iocp.session;

LogicServer::LogicServer()
{
}

LogicServer::~LogicServer()
{
    Stop();
}

bool LogicServer::Init(int maxSession)
{
    mMaxSession = maxSession;

	std::cout << std::format("LogicServer::Init: mMaxSession = {}\n", mMaxSession);

    SetDispatchCallback([this](unsigned int sessionId, std::span<const std::byte> packet) {
        mLogicManager.DisPatchPacket(sessionId, packet);
        });

    if (!Core::Init(maxSession))
    {
		std::cout << "Core Init failed\n";
        return false;
    }

    if (!mLogicManager.Init([this](int sessionId) {
        return this->GetSession(sessionId);
        }))
    {
		std::cout << "LogicManager Init failed\n";
        return false;
    }

    return true;
}

void LogicServer::Run()
{
    mLogicManager.Start();
}

void LogicServer::Stop()
{
    mLogicManager.Stop();
    Core::Stop();
}