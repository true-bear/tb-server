#pragma once
#include "pch.h"

#include "rioDispatcher.h"
#include "rioSession.h"
#include "rio.h"

#include <iostream>

bool RIOCompletionDispatcher::Start(RIO_CQ completionQueue)
{
    mCQ = completionQueue;
    mRunning = true;

	mThreadManager = std::make_unique<ThreadManager>();
	if (!mThreadManager)
	{
		std::cerr << "[RIO] Failed to create ThreadManager" << std::endl;
		return false;
	}

    mThreadManager->Run([this]() { this->Run(); });

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
        ULONG numResults = RIOCore::Table.RIODequeueCompletion(mCQ, results, MAX_RESULTS);

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

            // 아직 구분할 수단이 없으므로 무조건 Recv
            session->OnRecvCompleted(res.BytesTransferred);
        }
    }
}
