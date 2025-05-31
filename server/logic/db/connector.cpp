#include "pch.h"
#include <mysql/mysql.h>
#include "connector.h"

MYSQL* Connector::MySQLConnect(std::string_view host,
    std::string_view user,
    std::string_view password,
    std::string_view db,
    uint16_t port)
{
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) return nullptr;

    if (!mysql_real_connect(conn,
        std::string(host).c_str(),
        std::string(user).c_str(),
        std::string(password).c_str(),
        std::string(db).c_str(),
        port, nullptr, 0))
    {
        std::cerr << "MySQL connection failed\n";
        mysql_close(conn);
        return nullptr;
    }

    return conn;
}