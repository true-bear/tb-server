#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib,"mswsock.lib")

#define WSAAPI
#define INCL_WINSOCK_API_TYPEDEFS 1

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define LOG_INFO(who, format, ...)  Logger::Log(LogLevel::INFO, who, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(who, format, ...)  Logger::Log(LogLevel::WARN, who, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERR(who, format, ...) Logger::Log(LogLevel::ERR, who, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define LOG_INFOW(who, format, ...)  Logger::LogW(LogLevel::INFO, who, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARNW(who, format, ...)  Logger::LogW(LogLevel::WARN, who, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERRW(who, format, ...) Logger::LogW(LogLevel::ERR, who, __FILE__, __LINE__, format, ##__VA_ARGS__)



const wchar_t CATEGORY_NET[] = L"NETWORK";

const wchar_t PORT[] = L"port";
const wchar_t BACKLOG[] = L"backlog";
const wchar_t MAX_WORK_THEAD_CNT[] = L"workerCount";
const wchar_t MAX_SESSION_COUNT[] = L"maxSessionCount";
const wchar_t MAX_WAITING_COUNT[] = L"maxWaitingCount";
const wchar_t MAX_IOCP_EVENT_COUNT[] = L"maxIocpEventCount";


const int RECV_BUFFER_MAX_SIZE = 4096;
const int SEND_BUFFER_MAX_SIZE = 4096;

const int IOCP_EVENT_COUNT = 100;

const int PACKET_QUEUE_SIZE = 4096;