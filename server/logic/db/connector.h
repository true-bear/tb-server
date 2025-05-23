#pragma once
#include "pch.h"

class Connector
{
public:
    static MYSQL* MySQLConnect(const std::string& host,
        const std::string& user, 
        const std::string& password,
        const std::string& db,
        uint16_t port);

    //todo : redis

};

