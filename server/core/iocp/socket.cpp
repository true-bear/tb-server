#include "pch.h"

module;
module iocp.socket;

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <mswsock.h>
#include "../util/config.h"

bool SocketEx::Init()
{
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
		return false;
	
	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (mSocket == INVALID_SOCKET)
		return false;
	

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
		std::cout << "bind failed: Error=" << ret << std::endl;
		return false;
	}

	ret = ::listen(mSocket, SOMAXCONN);
	if (ret != 0)
	{
		std::cout << "listen failed: Error=" << ret << std::endl;
		return false;
	}

	return true;
}

bool SocketEx::Close()
{
	if (closesocket(mSocket) == SOCKET_ERROR)
	{
		std::cout << "closesocket failed" << std::endl;
		return false;
	}

	return true;
}

bool SocketEx::GetPeerName(sockaddr_in& addr) const {
	int addrlen = sizeof(addr);
	if (getpeername(mSocket, reinterpret_cast<sockaddr*>(&addr), &addrlen) == SOCKET_ERROR) {
		std::cout << "getpeername failed: Error=" << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

bool SocketEx::GetSocketInfo(WSAPROTOCOL_INFO& info) const {
	int len = sizeof(WSAPROTOCOL_INFO);
	if (getsockopt(mSocket, SOL_SOCKET, SO_PROTOCOL_INFO, reinterpret_cast<char*>(&info), &len) == SOCKET_ERROR) {
		std::cout << "getsockopt failed: Error=" << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

bool SocketEx::SetOption(int level, int optname, const void* optval, int optlen) {
	if (setsockopt(mSocket, level, optname, static_cast<const char*>(optval), optlen) == SOCKET_ERROR) {
		std::cout << std::format("setsockopt failed: Error={}\n", WSAGetLastError());
		return false;
	}

	return true;
}

void SocketEx::Detach()
{
	mSocket = INVALID_SOCKET;
}