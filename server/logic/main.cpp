#include "server/gameServer.h"

import util.singleton;
import util.conf; 

int main()
{
    if (!GameServer::Get().Init())
    {
		std::cout << "LogicServer initialization failed\n";
        return -1;
    }

    GameServer::Get().Run();

    std::string wait;
    std::getline(std::cin, wait);

    GameServer::Get().Stop();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}