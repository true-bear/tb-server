#pragma once
#pragma comment(lib, "ws2_32")
#pragma comment(lib,"mswsock.lib")


const wchar_t CATEGORY_NET[] = L"NETWORK";

const wchar_t PORT[] = L"port";
const wchar_t BACKLOG[] = L"backlog";
const wchar_t MAX_WORK_THEAD_CNT[] = L"workerCount";
const wchar_t MAX_SESSION_COUNT[] = L"maxSessionCount";
const wchar_t MAX_IOCP_EVENT_COUNT[] = L"maxIocpEventCount";


const int RECV_BUFFER_MAX_SIZE = 4096;
const int SEND_BUFFER_MAX_SIZE = 4096;

const int IOCP_EVENT_COUNT = 100;

const int PACKET_QUEUE_SIZE = 4096;