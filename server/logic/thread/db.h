#pragma once
#include "pch.h"
#include "..\db\dbJob.h"

class DBThread : public Singleton<DBThread>
{
public:
    DBThread();
    DBThread(std::string_view host,
        std::string_view user,
        std::string_view password,
        std::string_view db,
        uint16_t port);

    ~DBThread();

    void PushJob(std::shared_ptr<DBJob> job);
    void Start();
    void Stop();
    void RunThread();

private:
    std::mutex _mutex;
    std::condition_variable mCv;
    std::queue<std::shared_ptr<DBJob>> mQueue;
    std::atomic<bool> mRunning{ false };

    MYSQL* mConn = nullptr;
};