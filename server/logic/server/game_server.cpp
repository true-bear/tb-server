#include "game_server.h"
#include "../logic/logic.h"

import util.singleton;
import util.conf;
import iocp.session;

GameServer::GameServer()
{
}

GameServer::~GameServer()
{
    Stop();
}

bool GameServer::Init()
{
	const int port = Config::ReadInt(L"LOGIC", L"listenPort");
    const int maxSession = Config::ReadInt(L"LOGIC", L"maxSessionCount");
	const int worker = Config::ReadInt(L"LOGIC", L"workerCount");

	std::cout << std::format("LogicServer::Init: mMaxSession = {}\n", maxSession);

    SetDispatchCallback([this](unsigned int sessionId, std::span<const std::byte> packet) noexcept {
        mLogicManager.DispatchPacket(sessionId, packet);
        });

    if (!Core::Init(port, maxSession, worker))
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

void GameServer::Run()
{
	Core::Run();
    mLogicManager.Start();
}

void GameServer::Stop()
{
    mLogicManager.Stop();
    Core::Stop();
}