#include "pch.h"
#include "rio.h"

#ifndef WSAID_MULTIPLE_RIO_FUNCTION_TABLE
#define WSAID_MULTIPLE_RIO_FUNCTION_TABLE \
    { 0x8509e081, 0x96dd, 0x4005, { 0xb1, 0x4d, 0x65, 0xb8, 0x86, 0x85, 0x2d, 0x25 } }
#endif

RIO_EXTENSION_FUNCTION_TABLE RIOCore::Table{};

bool RIOCore::Init()
{
    GUID functionTableId = WSAID_MULTIPLE_RIO_FUNCTION_TABLE;
    DWORD bytes = 0;

    SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "[RIO] WSASocket failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (WSAIoctl(
        sock,
        SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
        &functionTableId,
        sizeof(functionTableId),
        &Table,
        sizeof(Table),
        &bytes,
        NULL,
        NULL) == SOCKET_ERROR)
    {
        std::cerr << "[RIO] WSAIoctl failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return false;
    }

    closesocket(sock);
    std::cout << "[RIO] RIOCore::Init() completed successfully." << std::endl;
    return true;
}
