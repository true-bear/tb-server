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

bool Gateway::InitAndConnect()
{
    // 메모 : 생성자에서 세팅하는게 좋을듯 너무 번거롭다
    SetDispatchCallback([this](unsigned id, std::span<const byte> frame) {
        this->Dispatch(id, frame);
        });

    const int maxSession = Config::ReadInt(L"NETWORK", L"maxSessionCount", 5000);
    if (!Core::Init(maxSession)) 
    {
        std::cout << "Gateway: Core::Init failed\n";
        return false;
    }

    const auto host = Config::ReadStr(L"GATEWAY", L"logicHost", L"127.0.0.1");
    const uint16_t port = static_cast<uint16_t>(Config::ReadInt(L"GATEWAY", L"logicPort", 9000));

    unsigned sid{};
    if (!Core::ConnectTo(host, port, ServerRole::Server, sid)) 
    {
        std::wcout << L"Gateway: ConnectTo(" << host << L":" << port << L") failed\n";
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
    Session* session = GetSession(mLogicSid);
    if (!session)
    {
        std::cerr << "Gateway:  session not exist\n";
        return;
    }

    std::vector<std::byte> relay;
    relay.resize(RELAY_HDR + frame.size());

    const uint32_t sid_net = htonl(clientSid);
    std::memcpy(relay.data(), &sid_net, RELAY_HDR);
    std::memcpy(relay.data() + RELAY_HDR, frame.data(), frame.size());

    if (!session->SendPacket(ToSpan(relay)))
        std::cerr << "Gateway: SendPacket to logic failed\n";
}

void Gateway::HandleFromLogic(unsigned /*logicSid*/, std::span<const std::byte> frame)
{
    if (frame.size() < RELAY_HDR) 
    {
        std::cerr << "Gateway: invalid relay frame (too small)\n";
        return;
    }

    uint32_t sid_net = 0;
    std::memcpy(&sid_net, frame.data(), RELAY_HDR);
    const unsigned targetClient = ntohl(sid_net);

    Session* session = GetSession(targetClient);
    if (!session)
    {
        std::cerr << std::format("Gateway: client {} not found for response\n", targetClient);
        return;
    }

    auto payload = frame.subspan(RELAY_HDR);

    if (!client->SendPacket(payload))
        std::cerr << "Gateway: SendPacket to client failed\n";
}