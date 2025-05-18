#include "pch.h"
#include "rioCore.h"
#include "rio.h"

RIOCore::RIOCore() {}
RIOCore::~RIOCore() { Stop(); }

bool RIOCore::Init(int port, int maxSession)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup 실패" << std::endl;
        return -1;
    }

    if (!RIOFns::Init())
    {
        std::cerr << "[RIOCore] RIO API 초기화 실패\n";
        return false;
    }

    mRecvCQ = RIOFns::Table.RIOCreateCompletionQueue(maxSession, nullptr);
    mSendCQ = RIOFns::Table.RIOCreateCompletionQueue(maxSession, nullptr);

    if (mRecvCQ == RIO_INVALID_CQ || mSendCQ == RIO_INVALID_CQ)
    {
        std::cerr << "[RIOCore] Completion Queue 생성 실패\n";
        return false;
    }

    for (int i = 0; i < maxSession; ++i)
    {
        mSessionPool.push_back(std::make_unique<RIOSession>());
    }

    mSocket = std::make_unique<RIOSocket>();
    if (!mSocket->Init(port))
    {
        std::cerr << "[RIOCore] Listen 소켓 초기화 실패\n";
        return false;
    }

    mDispatcher = std::make_unique<RIOCompletionDispatcher>();
    mDispatcher->Start(mRecvCQ);

    mRunning = true;
    return true;
}

void RIOCore::Run()
{
    std::cout << "[RIOCore] 클라이언트 대기 시작...\n";

    while (mRunning)
    {
        SOCKET clientSock = mSocket->Accept();
        if (clientSock == INVALID_SOCKET)
            continue;

        RIOSession* session = nullptr;
        for (auto& s : mSessionPool)
        {
            if (s->GetSocket() == INVALID_SOCKET)
            {
                session = s.get();
                break;
            }
        }

        if (!session)
        {
            std::cerr << "[RIOCore] 세션 풀 초과\n";
            closesocket(clientSock);
            continue;
        }

        if (!session->Init(clientSock, mRecvCQ, mSendCQ))
        {
            std::cerr << "[RIOCore] 세션 Init 실패\n";
            closesocket(clientSock);
            continue;
        }

        session->PostRecv();
        std::cout << "[RIOCore] 클라이언트 연결 완료\n";
    }
}

void RIOCore::Stop()
{
    mRunning = false;

    if (mDispatcher)
        mDispatcher->Stop();

    if (mSocket && mListenSocket != INVALID_SOCKET)
    {
        closesocket(mListenSocket);
        mListenSocket = INVALID_SOCKET;
    }

    std::cout << "[RIOCore] 서버 종료됨\n";
}


bool RIOCompletionDispatcher::Start(RIO_CQ completionQueue)
{
    mCQ = completionQueue;
    mRunning = true;
    mThread = std::thread(&RIOCompletionDispatcher::Run, this);
    return true;
}

void RIOCompletionDispatcher::Stop()
{
    mRunning = false;
    if (mThread.joinable())
        mThread.join();
}

void RIOCompletionDispatcher::Run()
{
    constexpr DWORD MAX_RESULTS = 32;
    RIORESULT results[MAX_RESULTS];

    while (mRunning)
    {
        ULONG numResults = RIOFns::Table.RIODequeueCompletion(mCQ, results, MAX_RESULTS);

        if (numResults == RIO_CORRUPT_CQ)
        {
            std::cerr << "[RIO] Completion Queue is corrupted" << std::endl;
            break;
        }

        if (numResults == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        for (ULONG i = 0; i < numResults; ++i)
        {
            RIORESULT& res = results[i];

            RIOSession* session = reinterpret_cast<RIOSession*>(res.RequestContext);
            if (!session)
                continue;

            if (res.Status != NO_ERROR)
            {
                std::cerr << "[RIO] I/O Error, Status = " << res.Status << std::endl;
                continue;
            }

            if (res.BytesTransferred == 0)
            {
                std::cerr << "[RIO] Peer closed connection" << std::endl;
                continue;
            }

            // Echo: 받은 데이터를 그대로 다시 보냄
            session->OnRecvCompleted(res.BytesTransferred);
            session->PostSend(session->GetReadPointer(), res.BytesTransferred);
        }
    }
}