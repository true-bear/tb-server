#include "game_server.h"
#include "../logic/logic.h"

import util.singleton;
import iocp.session;

GameServer::GameServer()
{
}

GameServer::~GameServer()
{
    Stop();
}

bool GameServer::Init(int maxSession)
{
    mMaxSession = maxSession;

	std::cout << std::format("LogicServer::Init: mMaxSession = {}\n", mMaxSession);

    SetDispatchCallback([this](unsigned int sessionId, std::span<const std::byte> packet) noexcept {
        mLogicManager.DispatchPacket(sessionId, packet);
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