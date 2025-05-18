#include "pch.h"
#include "rio.h"
#include "rioCore.h"

RIO_EXTENSION_FUNCTION_TABLE RIOFns::Table = {};
bool RIOFns::sInitialized = false;

#ifndef WSAID_MULTIPLE_RIO_FUNCTION_TABLE
#define WSAID_MULTIPLE_RIO_FUNCTION_TABLE \
    { 0x8509e081, 0x96dd, 0x4005, { 0xb1, 0x4d, 0x65, 0xb8, 0x86, 0x85, 0x2d, 0x25 } }
#endif

namespace
{
    std::unique_ptr<RIOCore> gRioCore;
}

namespace RIONetwork
{
    bool Init(int port, int maxSession)
    {
        gRioCore = std::make_unique<RIOCore>();

        if (!gRioCore->Init(port, maxSession))
        {
            std::cerr << "[RIO] Init 실패\n";
            return false;
        }

        std::cout << "[RIO] Init 완료\n";
        return true;
    }

    void Run()
    {
        if (!gRioCore)
        {
            std::cerr << "[RIO] Run 호출 전에 Init 필요\n";
            return;
        }

        gRioCore->Run();
    }

    void Stop()
    {
        if (gRioCore)
        {
            gRioCore->Stop();
            gRioCore.reset();
        }
    }
}



bool RIOFns::Init()
{
    if (sInitialized)
        return true;

    GUID functionTableId = WSAID_MULTIPLE_RIO_FUNCTION_TABLE;
    DWORD bytes = 0;

    SOCKET tempSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_REGISTERED_IO);
    if (tempSocket == INVALID_SOCKET)
    {
        std::cerr << "[RIOFns] WSASocket failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    if (WSAIoctl(tempSocket,
        SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
        &functionTableId,
        sizeof(functionTableId),
        &Table,
        sizeof(Table),
        &bytes,
        nullptr,
        nullptr) == SOCKET_ERROR)
    {
        std::cerr << "[RIOFns] WSAIoctl failed: " << WSAGetLastError() << std::endl;
        closesocket(tempSocket);
        return false;
    }

    closesocket(tempSocket);
    sInitialized = true;

    std::cout << "[RIOFns] RIO API 초기화 완료\n";
    return true;
}