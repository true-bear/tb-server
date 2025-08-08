#include "pch.h"
#include "server.h"
//#include <iostream>
//#include <format>

import util.singleton;
import util.conf; 
import common.define;

int main()
{
    const int maxSession = Config::Load(L"NETWORK", L"maxSessionCount");
	const int threadCount = Config::Load(L"NETWORK", L"workerCount");
    const int port = Config::Load(L"NETWORK", L"port");

    if (!LogicServer::Get().Init(maxSession))
    {
		std::cout << "LogicServer initialization failed\n";
        return -1;
    }

	std::cout << std::format("Server initialized with max sessions: {}, thread count: {}, port: {}\n", maxSession, threadCount, port);
	std::cout << std::format("Worker Thread Count: {}\n", threadCount);
	std::cout << std::format("Session Pool Size: {}\n", maxSession);
	std::cout << "IOCP Listen Port: " << port << std::endl;

    LogicServer::Get().Run();

    std::string wait;
    std::getline(std::cin, wait);

    LogicServer::Get().Stop();
    return 0;
}