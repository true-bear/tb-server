#include "pch.h"
#include <mysql/mysql.h>
#include "connector.h"

MYSQL* Connector::MySQLConnect(const std::string& host,
    const std::string& user,
    const std::string& password,
    const std::string& db,
    uint16_t port)
{
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) return nullptr;

    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
        db.c_str(), port, nullptr, 0)) 
    {
        std::cerr << "MySQL Connect Error: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return nullptr;
    }

    return conn;
}