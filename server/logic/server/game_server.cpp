#include "game_server.h"
#include "../logic/logic.h"

#include <WinSock2.h>   // ntohl/htonl
#include <ws2tcpip.h>
#include <cstring>      // std::memcpy
#include <functional>   // std::function
#include <cstdint>

import util.singleton;
import util.conf;
import iocp.session;

import <span>;
import <memory>;

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

    SetDispatchCallback([this](unsigned sid, std::span<const std::byte> pkt) noexcept {
        unsigned expected = 0;
        (void)mGatewaySid.compare_exchange_strong(expected, sid, std::memory_order_acq_rel);

        if (pkt.size() >= sizeof(uint32_t)) {
            uint32_t cidN = 0;
            std::memcpy(&cidN, pkt.data(), sizeof(cidN));
            const int clientId = static_cast<int>(ntohl(cidN));
            const auto inner = pkt.subspan(sizeof(uint32_t));

            mLogicManager.DispatchPacket(clientId, inner);
        }
        else 
        {
            mLogicManager.DispatchPacket(static_cast<int>(sid), pkt);
        }
        });

    if (!Core::Init(port, maxSession, worker)) 
    {
        std::cout << "Core Init failed\n";
        return false;
    }

    if (!mLogicManager.Init(
        [this](int /*ignored*/) -> Session* {
            const unsigned gw = mGatewaySid.load(std::memory_order_acquire);
            return this->GetSession(gw);
        },
        worker))
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