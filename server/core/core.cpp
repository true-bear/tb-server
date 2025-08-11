module;
#include <windows.h>
#include <iostream>
#include <format>
#include <ranges>

module core.engine;

import iocp;
import iocp.socket;
import iocp.session;
import thread.types;
import util.conf;

import <memory>;
import <mutex>;
import <atomic>;
import <span>;
import <unordered_map>;
import <functional>;

Core::Core() = default;

Core::~Core()
{
    Stop();
}

bool Core::Init(int maxSession)
{
    if (maxSession <= 0)
        return false;
    
    mMaxSession = maxSession;

    if (!mListenSocket.Init() || !mListenSocket.BindAndListen())
        return false;

    SYSTEM_INFO sysInfo;
    ::GetSystemInfo(&sysInfo);
    const int workerCount = static_cast<int>(sysInfo.dwNumberOfProcessors) * 2;

    mWorkerCnt = workerCount;

    if (!CreateNewIocp(workerCount))
        return false;

    if (!AddDeviceListenSocket(mListenSocket.GetSocket()))
        return false;

    if (!CreateSessionPool())
        return false;
	
    mIsRunThread = true;

    if (!mDispatchCallback)
        return false;

    return true;
}

bool Core::CreateSessionPool()
{
    auto ranges = std::views::iota(0, mMaxSession);
    for (int i : ranges)
    {
        auto session = std::make_unique<Session>();
        if (!session)
            return false;

        session->SetUniqueId(i);

        const auto& listenSocket = mListenSocket.GetSocket();
        if (!session->AcceptReady(listenSocket, i))
            return false;

        mSessionPool.emplace(i, std::move(session));
    }

    return true;
}

void Core::Run()
{
    auto ranges = std::views::iota(0, mWorkerCnt);
    for (int i : ranges)
    {
        auto worker = std::make_unique<Worker>(
            static_cast<IEventHandler*>(this), 
            static_cast<IIoHandler*>(this), 
            "worker", 
            i, 
            ThreadType::Worker);
        worker->Start();
        mWorkers.emplace_back(std::move(worker));
    }

    std::cout << std::format("Core::Start: mMaxSession = {}\n", mMaxSession);

}
void Core::Stop()
{
    mIsRunThread.store(false, std::memory_order_release);

    for (const auto& _ : mWorkers)
    {
		Iocp::PQCS(0, 0, nullptr);
    }

    for (auto& worker : mWorkers)
    {
        worker->Stop();
    }

    mSessionPool.clear();
}

bool Core::IsRunThread() const
{
    return mIsRunThread.load(std::memory_order_acquire);
}

Session* Core::GetSession(unsigned int uID) const
{
    if (const auto it = mSessionPool.find(uID); it != mSessionPool.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void Core::SetDispatchCallback(std::function<void(unsigned int, std::span<const std::byte>)> callback)
{
    mDispatchCallback = std::move(callback);
}

void Core::GetIocpEvents(IocpEvents& events, unsigned long timeout)
{
    GQCSEx(events, timeout);
}

void Core::OnRecv(unsigned int uID, unsigned long ioSize)
{
    auto session = GetSession(uID);
    if (!session)
    {
        std::cout << std::format("OnRecv: session not found for id: {}\n", uID);
        return;
    }

    auto recvBuffer = session->GetRecvBuffer();
    if (!recvBuffer)
    {
        std::cout << "OnRecv: recvBuffer null id:" << uID << std::endl;
        return;
    }

    session->RecvPacket(ioSize);

    while (true)
    {
        const size_t storedSize = recvBuffer->ReadableSize();
        if (storedSize < sizeof(uint16_t))
            break;

        uint16_t packetSize = 0;
        std::span<std::byte> headerView(reinterpret_cast<std::byte*>(&packetSize), sizeof(packetSize));
        if (!recvBuffer->Peek(headerView.data(), sizeof(packetSize)))
            return;

        packetSize = ntohs(packetSize);
        if (packetSize == 0) return;

        if (storedSize < sizeof(uint16_t) + packetSize)
            break;

        recvBuffer->MoveReadPos(sizeof(packetSize));

        std::span<const std::byte> packetData{ recvBuffer->ReadPtr(), packetSize };

        mDispatchCallback(uID, packetData);

        recvBuffer->MoveReadPos(packetSize);
    }


    if (!session->RecvReady())
    {
        std::cout << std::format("OnRecv: RecvReady failed for session {}\n", uID);
        OnClose(uID);
    }
}

void Core::OnAccept(unsigned int uID, unsigned long long completekey)
{
    auto session = GetSession(uID);
    if (!session || completekey != 0)
    {
        std::cout << std::format("OnAccept: invalid session or completeKey: {}\n", uID);
        return;
    }

    if (!AddDeviceRemoteSocket(session))
    {
        OnClose(uID);
        return;
    }

    auto listenSocket = GetListenSocket();

    if (!session->AcceptFinish(listenSocket))
    {
        OnClose(uID);
        return;
    }

    session->RecvReady();  
}

void Core::OnClose(unsigned int uID)
{
    std::lock_guard<std::mutex> lock(mSessionLock);

    auto session = GetSession(uID);
    if (!session)
    {
        std::cout << std::format("OnClose: session not found for uID: {}\n", uID);
		return;
    }
    

    session->DisconnectFinish();
    session->Init();

    if (!session->AcceptReady(GetListenSocket(), uID))
    {
        std::cout << std::format("OnClose: AcceptReady failed for session {}\n", uID);
        return;
    }

    std::cout << std::format("OnClose: session {} disconnected\n", uID);
    return;
}

void Core::OnSend(unsigned int uID, unsigned long ioSize)
{
    auto session = GetSession(uID);
    if (!session)
        return;

    auto* sendBuffer = session->GetSendBuffer();
    if (sendBuffer)
    {
        sendBuffer->MoveReadPos(static_cast<size_t>(ioSize));
    }
}
