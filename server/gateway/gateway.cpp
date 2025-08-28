#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <WinSock2.h>
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

void Gateway::Run()
{
    Core::Run();
    mSendThread->Start();
}

void Gateway::Stop()
{
    if (mSendThread) 
    {
        mSendThread->Stop();
        mSendThread.reset();
    }

    Core::Stop();
}
bool Gateway::InitAndConnect(const int sessionCount, const int worker, const int port)
{
    SetDispatchCallback([this](std::uint64_t id, std::span<const byte> frame) {
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

    mSendThread = std::make_unique<SendThread>(
        "GatewaySendThread",
        [this](std::uint64_t sid) { return this->GetSession(sid); },
        mSendQueue
    );

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

void Gateway::HandleFromClient(std::uint64_t clientSid, std::span<const std::byte> frame)
{
    if (!GetSession(mLogicSid)) { std::cerr << "GW: logic not ready\n"; return; }

    const std::uint32_t idN = htonl(static_cast<std::uint32_t>(clientSid));
    std::vector<std::byte> relay(sizeof(std::uint32_t) + frame.size());

    std::copy_n(reinterpret_cast<const std::byte*>(&idN), sizeof(idN), relay.begin());
    std::copy(frame.begin(), frame.end(), relay.begin() + sizeof(idN));


    EnqueueSend(mLogicSid, { relay.data(), relay.size() });
    mLastClientSid.store(clientSid, std::memory_order_release);
}

void Gateway::HandleFromLogic(std::span<const std::byte> frame)
{
    if (frame.size() < RELAY_HDR) 
        return;

    std::uint32_t sidNet = 0; std::memcpy(&sidNet, frame.data(), RELAY_HDR);
    unsigned target = ntohl(sidNet);

    if (!GetSession(target)) 
    {
        unsigned fb = mLastClientSid.load(std::memory_order_acquire);
        if (!fb || !GetSession(fb)) 
        { 
            std::cerr << "GW: client not found\n"; 
            return; 
        }

        target = fb;
    }

    auto payload = frame.subspan(RELAY_HDR);
    EnqueueSend(target, payload);
}

bool Gateway::EnqueueSend(uint64_t sid, std::span<const std::byte> data)
{
    if (data.size() > GW_MAX_PACKET)
        return false;

    GatewaySendNode n{};
    n.sessionId = sid;
    n.size = static_cast<uint32_t>(data.size());
    std::memcpy(n.data.data(), data.data(), data.size());

    if(mSendQueue.push(n))
    {
        if (sid == mLogicSid)
            std::cout << std::format("ENQ GW->LOGIC : bytes={}\n", n.size);
        else
            std::cout << std::format("ENQ LOGIC->GW : bytes={}\n", n.size);
    }

    return true;
}