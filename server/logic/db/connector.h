#pragma once
#include "pch.h"

class Connector
{
public:
    static MYSQL* MySQLConnect(std::string_view host,
        std::string_view user,
        std::string_view password,
        std::string_view db,
        uint16_t port);

    //todo : redis

};

