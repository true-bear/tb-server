#include "pch.h"
#include "server.h"

int main()
{
    const int maxSession = Config::Load(CATEGORY_NET, MAX_SESSION_COUNT);
	const int maxWaiting = Config::Load(CATEGORY_NET, MAX_WAITING_COUNT);
    const int backlog = Config::Load(CATEGORY_NET, BACKLOG);
	const int threadCount = Config::Load(CATEGORY_NET, MAX_WORK_THEAD_CNT);
    const int port = Config::Load(CATEGORY_NET, PORT);


    if (!LogicServer::Get().Init(maxSession, maxWaiting, backlog))
    {
        LOG_ERR("main()", "server init");
        return -1;
    }


    LOG_INFO("SERVER INIT","");
    LOG_INFO("Worker Thread Count: {}","", threadCount);
    LOG_INFO("Session Pool Size: {}","", maxSession);
    LOG_INFO("IOCP Listen Port: {}","", port);

    LogicServer::Get().Run();

    std::string wait;
    std::getline(std::cin, wait);

    LogicServer::Get().Stop();
    return 0;
}