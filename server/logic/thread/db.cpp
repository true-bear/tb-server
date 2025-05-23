#include "pch.h"
#include "db.h"
#include "..\db\connector.h"
#include "mysql/mysql.h"

DBThread::DBThread()
{
}
DBThread::DBThread(const std::string& host,
    const std::string& user,
    const std::string& password,
    const std::string& db,
    uint16_t port)
{
    mConn = Connector::MySQLConnect(host, user, password, db, port);
}

DBThread::~DBThread()
{
    Stop();
    if (mConn)
        mysql_close(mConn);

    //TODO : ťó���� ���־�� �ҵ�
}

void DBThread::PushJob(std::shared_ptr<DBJob> job)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);
        mQueue.push(job);
    }
    mCv.notify_one();
}

void DBThread::Start()
{
    mRunning.store(true);
}

void DBThread::Stop()
{
    mRunning.store(false);
    mCv.notify_all();
}


//MEMO : �̰� ������ ���� ������ ó���ؾ� ���� ������? �Ҽ��� mod����
void DBThread::RunThread()
{
    while (mRunning)
    {
        std::shared_ptr<DBJob> job = nullptr;

        {
            std::unique_lock<std::mutex> lock(_mutex);
            mCv.wait(lock, [&]() { return !mQueue.empty() || !mRunning; });

            if (!mRunning && mQueue.empty())
            {
                break;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            job = mQueue.front();
            mQueue.pop();
        }

        if (job && mConn)
            job->Execute(mConn);
    }
}