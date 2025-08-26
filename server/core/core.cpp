module;
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>
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

bool Core::Init(const int listenPort, const int maxSession, const int workerCount)
{
    mMaxSession = maxSession;
    mWorkerCnt = workerCount;

    if (!mListenSocket.Init())
        return false;

    if (!mListenSocket.BindAndListen(listenPort))
        return false;

    if (!CreateNewIocp(mWorkerCnt))
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

        auto listenSocket = mListenSocket.GetSocket();
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


    int port = Config::ReadInt(L"LOGIC", L"listenPort");
    std::cout << std::format("Core::Start: port = {}\n", port);
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

Session* Core::GetSession(const std::uint64_t uID) const
{
    if (const auto it = mSessionPool.find(uID); it != mSessionPool.end())
    {
        return it->second.get();
    }

    return nullptr;
}

void Core::GetIocpEvents(IocpEvents& events, const unsigned long timeout)
{
    GQCSEx(events, timeout);
}

void Core::OnRecv(const std::uint64_t uID, const std::uint32_t ioSize)
{
    auto session = GetSession(uID);
    if (!session)
        return;
    

    auto recvBuffer = session->GetRecvBuffer();
    if (!recvBuffer)
        return;

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
        OnClose(uID);
    
}

void Core::OnAccept(const std::uint64_t uID, const std::uint64_t key)
{
    auto session = GetSession(uID);
    if (!session || key != 0)
        return;
    

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

void Core::OnClose(const std::uint64_t uID)
{
    std::lock_guard<std::mutex> lock(mSessionLock);

    auto session = GetSession(uID);
    if (!session)
		return;
    
    const bool sessionType = (session->GetRole() == ServerRole::Server);

    session->DisconnectFinish();
    session->Reset();

	if (sessionType)
	{
		mSessionPool.erase(uID);
	}
	else
	{
        if (!session->AcceptReady(GetListenSocket(), uID))
            return;
	}


    std::cout << std::format("OnClose: session {} disconnected\n", uID);
    return;
}

void Core::OnSend(const std::uint64_t uID, const std::uint32_t ioSize)
{
    if (auto* s = GetSession(uID)) 
        s->SendFinish(static_cast<size_t>(ioSize));
}

void Core::OnConnect(const std::uint64_t uID)
{
    if (auto* s = GetSession(uID)) 
    {
        DWORD flags = 0, bytes = 0;
        BOOL ok = WSAGetOverlappedResult(
            s->GetRemoteSocket(), s->GetConnectOv(), &bytes, FALSE, &flags);

        if (!ok) 
        {
            OnClose(uID);
            return;
        }

        if (!s->SetFinishConnectContext()) 
        {
            OnClose(uID);
            return;
        }

        if (!s->RecvReady()) 
        {
            OnClose(uID);
            return;
        }

        std::cout << "[CONNECT] OK sid=" << uID << "\n";
    }
}

bool Core::ConnectTo(const std::wstring& ip, uint16_t port, ServerRole role, const std::uint64_t logicSessionId)
{
    SocketEx tmp;
    if (!tmp.Init())
        return false;


    auto newSession = std::make_unique<Session>();
    if (!newSession) 
        return false;

    newSession->SetUniqueId(logicSessionId);
    newSession->SetRole(role);
    newSession->AttachSocket(std::move(tmp));

    Session* connSession = newSession.get();
    mSessionPool.emplace(logicSessionId, std::move(newSession));

    if (!AddDeviceRemoteSocket(connSession))
    {
        mSessionPool.erase(logicSessionId);
        return false;
    }

    connSession->PrepareConnectOv();

    if (!connSession->GetRemoteSock().ConnectEx(ip.c_str(), port, connSession->GetConnectOv()))
    {
        const int e = WSAGetLastError();
        if (e != WSA_IO_PENDING) 
        {
            mSessionPool.erase(logicSessionId);
            return false;
        }
    }

    return true;
}