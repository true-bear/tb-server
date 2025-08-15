#include <iostream>
#include "proto/message.pb.h"
#include "gateway.h"

import core.engine;

int main()
{
	const int maxSession = static_cast<int>(Config::ReadInt(L"GATEWAY", L"maxSessionCount", 1000));
    const int port       = static_cast<int>(Config::ReadInt(L"GATEWAY", L"listenPort", 9000));
	const int worker     = static_cast<int>(Config::ReadInt(L"GATEWAY", L"workerCount", 3));

    if (!Gateway::Get().InitAndConnect(maxSession, worker, port))
    {
        std::cerr << "Gateway init failed\n";
        return 1;
    }


    Gateway::Get().Run();
    std::cout << "Gateway running. .\n";

    std::string wait;
    std::getline(std::cin, wait);

    Gateway::Get().Stop();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}