#pragma once

#include "pch.h"
#include "Socket.h"
#include "config.h"
#include "logger.h"

bool SocketEx::Init()
{
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		LOG_ERR("WSAStartup", "ret:{}", ret);
		return false;
	}

	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (mSocket == INVALID_SOCKET)
	{
		LOG_ERR("WSASocket", "");
		return false;
	}

	return true;
}

bool SocketEx::BindAndListen()
{
	int port = Config::Load(CATEGORY_NET, PORT);

	SOCKADDR_IN		stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(port);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = ::bind(mSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (ret != 0)
	{
		LOG_ERR("::bind", "ret:{}", ret);
		return false;
	}

	ret = ::listen(mSocket, SOMAXCONN);
	if (ret != 0)
	{
		LOG_ERR("::listen", "ret:{}", ret);
		return false;
	}

	return true;
}

bool SocketEx::Close()
{
	if (closesocket(mSocket) == SOCKET_ERROR)
	{
		LOG_ERR("closesocket", "");
		return false;
	}

	return true;
}

bool SocketEx::GetPeerName(sockaddr_in& addr) const {
	int addrlen = sizeof(addr);
	if (getpeername(mSocket, reinterpret_cast<sockaddr*>(&addr), &addrlen) == SOCKET_ERROR) {
		LOG_ERR("getpeername", "Error: {}", WSAGetLastError());
		return false;
	}

	return true;
}

bool SocketEx::GetSocketInfo(WSAPROTOCOL_INFO& info) const {
	int len = sizeof(WSAPROTOCOL_INFO);
	if (getsockopt(mSocket, SOL_SOCKET, SO_PROTOCOL_INFO, reinterpret_cast<char*>(&info), &len) == SOCKET_ERROR) {
		LOG_ERR("getsockopt", "Error getting socket info: {}", WSAGetLastError());
		return false;
	}

	return true;
}

bool SocketEx::SetOption(int level, int optname, const void* optval, int optlen) {
	if (setsockopt(mSocket, level, optname, static_cast<const char*>(optval), optlen) == SOCKET_ERROR) {
		LOG_ERR("setsockopt", "Error: {}", WSAGetLastError());
		return false;
	}

	return true;
}

void SocketEx::Detach()
{
	mSocket = INVALID_SOCKET;
}