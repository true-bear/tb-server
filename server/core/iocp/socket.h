#pragma once
#include "../pch.h"

class SocketEx
{
public:
	bool Init();
	bool Close();

	bool BindAndListen();

	void Detach();

	bool SetSocket(SOCKET& newSocket) { mSocket = newSocket; return true; }
	bool SetOption(int level, int optname, const void* optval, int optlen);

	bool GetPeerName(sockaddr_in& addr) const;
	const SOCKET& GetSocket() const { return mSocket; }
	bool GetSocketInfo(WSAPROTOCOL_INFO& info) const;

private:
	SOCKET mSocket{ INVALID_SOCKET };
};