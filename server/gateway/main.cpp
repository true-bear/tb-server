#include <iostream>
#include "proto/message.pb.h"
#include "gateway.h"

import core.engine;

int main()
{
    if (!Gateway::Get().InitAndConnect())
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