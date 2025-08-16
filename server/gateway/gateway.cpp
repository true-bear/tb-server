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

static std::span<const std::byte> ToSpan(const std::vector<std::byte>& v) {
    return { v.data(), v.size() };
}

static std::span<const std::byte> ToSpan(const std::string& s) {
    return { reinterpret_cast<const std::byte*>(s.data()), s.size() };
}

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
    const int  logicPort = Config::ReadInt(L"GATEWAY", L"logicPort", 9000);

    unsigned sid{};
    if (!Core::ConnectTo(host, static_cast<uint16_t>(logicPort), ServerRole::Server, sid)) 
    {
        std::wcout << L"Gateway: ConnectTo(" << host << L":" << logicPort << L") failed\n";
        return false;
    }

    mLogicSid = sid;

    return true;
}

void Gateway::Dispatch(unsigned id, std::span<const byte> frame)
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
        HandleFromLogic(id, frame);
    }
}

void Gateway::HandleFromClient(unsigned clientSid, std::span<const std::byte> frame)
{
    Session* logic = GetSession(mLogicSid);
    if (!logic) { std::cerr << "GW: logic not ready\n"; return; }

    const uint32_t idN = htonl(clientSid);

    std::vector<std::byte> relay(4 + frame.size());
    std::memcpy(relay.data() + 0, &idN, 4);
    std::memcpy(relay.data() + 4, frame.data(), frame.size());

    if (!logic->SendPacket({ relay.data(), relay.size() })) {
        std::cerr << "GW: SendPacket to logic failed\n";
    }

    mLastClientSid.store(clientSid, std::memory_order_release);
}


void Gateway::HandleFromLogic(unsigned /*logicSid*/, std::span<const std::byte> frame)
{
    if (frame.size() < RELAY_HDR) return;

    uint32_t sidNet = 0;
    std::memcpy(&sidNet, frame.data(), RELAY_HDR);
    unsigned target = ntohl(sidNet);

    Session* cli = GetSession(target);
    if (!cli) {
        unsigned fb = mLastClientSid.load(std::memory_order_acquire);
        if (fb == 0 || !(cli = GetSession(fb))) {
            std::cerr << "GW: client not found (target=" << target << ")\n";
            return;
        }
        target = fb;
    }

    auto payload = frame.subspan(RELAY_HDR); // [2B type][4B len][body]
    (void)cli->SendPacket(payload);
}


//void Gateway::HandleFromClient(unsigned clientSid, std::span<const std::byte> frame)
//{
//    Session* session = GetSession(mLogicSid);
//    if (!session)
//    {
//        std::cerr << "Gateway:  session not exist\n";
//        return;
//    }
//
//    std::vector<std::byte> relay;
//    relay.resize(RELAY_HDR + frame.size());
//
//    const uint32_t sid_net = htonl(clientSid);
//    std::memcpy(relay.data(), &sid_net, RELAY_HDR);
//    std::memcpy(relay.data() + RELAY_HDR, frame.data(), frame.size());
//
//    if (!session->SendPacket(ToSpan(relay)))
//    {
//        const int err = WSAGetLastError();
//        std::cout << "SendPacket: Write failed uid:" << clientSid
//            << " sz:" << relay.size() << " wsa=" << err << "\n";
//    }
//}

//void Gateway::HandleFromLogic(unsigned /*logicSid*/, std::span<const std::byte> frame)
//{
//    if (frame.size() < RELAY_HDR) 
//    {
//        std::cerr << "Gateway: invalid relay frame (too small)\n";
//        return;
//    }
//
//    uint32_t sid_net = 0;
//    std::memcpy(&sid_net, frame.data(), RELAY_HDR);
//    const unsigned targetClient = ntohl(sid_net);
//
//    Session* session = GetSession(targetClient);
//    if (!session)
//    {
//        std::cerr << std::format("Gateway: client {} not found for response\n", targetClient);
//        return;
//    }
//
//    auto payload = frame.subspan(RELAY_HDR);
//
//    if (!session->SendPacket(payload))
//        std::cerr << "Gateway: SendPacket to client failed\n";
//}