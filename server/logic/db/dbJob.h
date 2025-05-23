#pragma once
#include "pch.h"
#include "mysql/mysql.h"

using RowData = std::vector<std::string>;
using DBCallback = std::function<void(bool, const std::vector<RowData>&)>;

class DBJob
{
public:
    DBJob(const std::string& query, DBCallback callback);

    void Execute(MYSQL* conn);
    const std::string& GetQuery() const;

private:
    std::string _query;
    DBCallback _callback;
};