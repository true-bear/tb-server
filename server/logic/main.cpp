#include "pch.h"
#include "server.h"
#include "thread/db.h" // 임시

int main()
{
    const int maxSession = Config::Load(CATEGORY_NET, MAX_SESSION_COUNT);
	const int maxWaiting = Config::Load(CATEGORY_NET, MAX_WAITING_COUNT);
	const int threadCount = Config::Load(CATEGORY_NET, MAX_WORK_THEAD_CNT);
    const int port = Config::Load(CATEGORY_NET, PORT);

    if (!LogicServer::Get().Init(maxSession, maxWaiting))
    {
        LOG_ERR("main()", "server init");
        return -1;
    }

    LOG_INFO("SERVER INIT","");
    LOG_INFO("Worker Thread Count: {}","", threadCount);
    LOG_INFO("Session Pool Size: {}","", maxSession);
    LOG_INFO("IOCP Listen Port: {}","", port);

    LogicServer::Get().Run();


    DBThread* gDBThread = new DBThread("127.0.0.1", "admin", "123123", "test", 3306);

    gDBThread->Start();

    auto job = std::make_shared<DBJob>(
        "SELECT uid FROM users",
        [](bool success, const std::vector<RowData>& rows)
        {
            if (!success) return;
            for (const auto& row : rows)
                std::cout << "id: " << row[0] << "\n";

        });

    gDBThread->PushJob(job);


    std::string wait;
    std::getline(std::cin, wait);

    LogicServer::Get().Stop();
    return 0;
}