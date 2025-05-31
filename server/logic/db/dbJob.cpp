#include "pch.h"
#include "dbJob.h"

DBJob::DBJob(std::string_view query, DBCallback callback)
    : _query(query), _callback(std::move(callback))
{
}

std::string_view DBJob::GetQuery() const
{
    return _query;
}

void DBJob::Execute(MYSQL* conn)
{
    bool success = false;
    std::vector<RowData> rows;

    if (mysql_query(conn, _query.c_str()) == 0)
    {
        success = true;
        MYSQL_RES* result = mysql_store_result(conn);
        if (result)
        {
            int num_fields = mysql_num_fields(result);
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result)))
            {
                RowData rowData;
                for (int i = 0; i < num_fields; ++i)
                    rowData.push_back(row[i] ? row[i] : "");
                rows.push_back(std::move(rowData));
            }
            mysql_free_result(result);
        }
    }
    else
    {
        std::cerr << "Query failed: " << mysql_error(conn) << "\n";
    }

    if (_callback)
        _callback(success, rows);
}