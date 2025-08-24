#include <windows.h>
#include <string>
#include <iostream>
#include <format>
#include <locale>
#include <codecvt>

#include "proto/message.pb.h"
#include "gateway.h"

import core.engine;
import util.conf;
import common.define;
import iocp.session;

import <span>;

using std::byte;

bool Gateway::InitAndConnect(const int sessionCount, const int worker, const int port)
{
    SetDispatchCallback([this](unsigned id, std::span<const byte> frame) {
        this->Dispatch(id, frame);
        });

    if (!Core::Init(port, sessionCount, worker))
    {
        std::cout << "Gateway: Core::Init failed\n";
        return false;
    }

    const auto host = Config::ReadStr(L"GATEWAY", L"logicHost", L"127.0.0.1");
    const int logicPort = Config::ReadInt(L"GATEWAY", L"logicPort", 9000);
    const int logicSession = Config::ReadInt(L"GATEWAY", L"logicSession", 5000);

    if (!Core::ConnectTo(host, static_cast<uint16_t>(logicPort), ServerRole::Server, logicSession))
    {
        std::wcout << L"Gateway: ConnectTo(" << host << L":" << logicPort << L") failed\n";
        return false;
    }

    mLogicSid = logicSession;

    return true;
}

void Gateway::Dispatch(const std::uint64_t id, std::span<const byte> frame)
{
    Session* session = GetSession(id);
    if (!session)
        return;

    if (session->GetRole() == ServerRole::Client)
    {
        HandleFromClient(id, frame);
    }
    else 
    {
        HandleFromLogic(frame);
    }
}

void Gateway::HandleFromClient(const std::uint64_t clientSid, std::span<const std::byte> frame)
{
    Session* logic = GetSession(mLogicSid);
    if (!logic) 
    { 
        std::cerr << "GW: logic not ready\n"; 
        return; 
    }

    const std::uint32_t idN = htonl(clientSid);

    std::vector<std::byte> relay(sizeof(std::uint32_t) + frame.size());
    std::memcpy(relay.data(), &idN, sizeof(std::uint32_t));
    std::memcpy(relay.data() + sizeof(std::uint32_t), frame.data(), frame.size());

    if (!logic->SendPacket({ relay.data(), relay.size() })) 
    {
        std::cerr << "GW: SendPacket to logic failed\n";
    }

    mLastClientSid.store(clientSid, std::memory_order_release);
}


void Gateway::HandleFromLogic(std::span<const std::byte> frame)
{
    if (frame.size() < RELAY_HDR) 
        return;

    std::uint32_t sidNet = 0;
    std::memcpy(&sidNet, frame.data(), RELAY_HDR);
    unsigned target = ntohl(sidNet);

    Session* cli = GetSession(target);
    if (!cli) 
    {
        unsigned fb = mLastClientSid.load(std::memory_order_acquire);
        if (fb == 0 || !(cli = GetSession(fb))) 
        {
            std::cerr << "GW: client not found (target=" << target << ")\n";
            return;
        }
        target = fb;
    }

    auto payload = frame.subspan(RELAY_HDR);
    (void)cli->SendPacket(payload);
}