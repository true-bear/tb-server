#pragma once
#include "pch.h"
#include "mysql/mysql.h"

using RowData = std::vector<std::string>;
using DBCallback = std::function<void(bool, const std::vector<RowData>&)>;

class DBJob
{
public:
    DBJob(std::string_view query, DBCallback callback);

    void Execute(MYSQL* conn);
    std::string_view GetQuery() const;

private:
    std::string _query;
    DBCallback _callback;
};