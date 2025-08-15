
module;
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <iostream>
#include <format>

module iocp.session;
import iocp.socket;
import util.roundbuffer;
import common.define;

import <cstdint>;
import <memory>;
import <span>;
import <vector>;
import <functional>;

Session::Session()
{
	ZeroMemory(&mRecvOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mSendOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mAcceptOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mConnectOverEx, sizeof(OverlappedIoEx));

	mRecvOverEx.mIOType = IO_TYPE::RECV;
	mSendOverEx.mIOType = IO_TYPE::SEND;
	mAcceptOverEx.mIOType = IO_TYPE::ACCEPT;
	mConnectOverEx.mIOType = IO_TYPE::CONNECT;
}

Session::~Session() = default;

void Session::Reset()
{
	ZeroMemory(&mRecvOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mSendOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mAcceptOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mConnectOverEx, sizeof(OverlappedIoEx));

	mRecvOverEx.mIOType = IO_TYPE::RECV;
	mSendOverEx.mIOType = IO_TYPE::SEND;
	mAcceptOverEx.mIOType = IO_TYPE::ACCEPT;
	mConnectOverEx.mIOType = IO_TYPE::CONNECT;

	mRecvBuffer->Reset();
	mSendBuffer->Reset();

	std::scoped_lock lk(mSendLock);

	mSendPending.store(false, std::memory_order_release);

	ZeroMemory(mAcceptBuf, sizeof(mAcceptBuf));
}


void Session::DisconnectFinish()
{
	if (auto socket = mRemoteSock.GetSocket(); socket != INVALID_SOCKET)
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}
}

bool Session::AcceptReady(const SOCKET& listenSock, const int uID)
{
	SOCKET socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP,
		NULL, 0, WSA_FLAG_OVERLAPPED);
	if (socket == INVALID_SOCKET)
		return false;

	unsigned long bytes = 0;
	unsigned long flags = 0;

	mAcceptOverEx.mUID = uID;
	mAcceptOverEx.mWsaBuf = { 0, nullptr };
	mAcceptOverEx.mIOType = IO_TYPE::ACCEPT;

	auto ret = AcceptEx(
		listenSock,
		socket,
		mAcceptBuf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&bytes,
		(LPWSAOVERLAPPED) & (mAcceptOverEx)
	);

	if (!ret && WSAGetLastError() != WSA_IO_PENDING)
		return false;

	return mRemoteSock.SetSocket(socket);
}

bool Session::AcceptFinish(const SOCKET& listenSocket)
{
	auto remoteSocket = mRemoteSock.GetSocket();

	if (setsockopt(remoteSocket,
		SOL_SOCKET,
		SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&listenSocket,
		sizeof(listenSocket)) == SOCKET_ERROR)
	{
		std::cout << std::format("AcceptFinish: setsockopt failed : {}\n", WSAGetLastError());
		return false;
	}

	sockaddr_in clientAddr;
	if (!mRemoteSock.GetPeerName(clientAddr))
		return false;

	int flag = 1;
	if (!mRemoteSock.SetOption(IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)))
	{
		std::cout << std::format("AcceptFinish: Failed to set TCP_NODELAY option\n");
	}


	SOCKADDR_IN* localAddr = nullptr;
	int remoteLen = sizeof(SOCKADDR_IN);
	int localLen = sizeof(SOCKADDR_IN);


	GetAcceptExSockaddrs(
		mAcceptBuf,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		(LPSOCKADDR*)&localAddr,
		&localLen,
		(LPSOCKADDR*)&clientAddr,
		&remoteLen);

	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);

	return true;
}

bool Session::RecvReady()
{
	std::byte* writePtr = mRecvBuffer->WritePtr();
	size_t writeSize = mRecvBuffer->WritableSize();

	mRecvOverEx.mUID = mUID;

	mRecvOverEx.mWsaBuf.buf = reinterpret_cast<char*>(writePtr);
	mRecvOverEx.mWsaBuf.len = static_cast<ULONG>(writeSize);

	DWORD recvBytes = 0;
	DWORD flags = 0;

	int ret = WSARecv(
		mRemoteSock.GetSocket(),
		&mRecvOverEx.mWsaBuf,
		1,
		&recvBytes,
		&flags,
		reinterpret_cast<LPWSAOVERLAPPED>(&mRecvOverEx),
		nullptr
	);

	if (ret == SOCKET_ERROR)
	{
		const int err = WSAGetLastError();
		if (err != WSA_IO_PENDING)
			return false;
	}

	return true;
}


bool Session::RecvPacket(unsigned long ioSize)
{
	if (ioSize == 0 || ioSize > 4096) //todo : 하드 코딩 제거
	{
		std::cout << std::format("RecvPacket: Invalid ioSize: {} for id: {}\n", ioSize, mUID);
		return false;
	}

	mRecvBuffer->MoveWritePos(ioSize);

	return true;
}

char* Session::GetRecvOverlappedBuffer() const
{
	return mRecvOverEx.mWsaBuf.buf;
}

bool Session::SendPacket(std::span<const std::byte> data)
{
	const uint16_t sizeBE = htons(static_cast<uint16_t>(data.size()));
	const auto* hdr = reinterpret_cast<const std::byte*>(&sizeBE);
	const std::span<const std::byte> headerSpan{ hdr, sizeof(sizeBE) };

	std::scoped_lock lk(mSendLock);

	if (!mSendBuffer->Write(headerSpan) || !mSendBuffer->Write(data)) 
	{
		std::cout << std::format("SendPacket: Write failed uid:{} sz:{}\n", mUID, data.size());
		return false;
	}

	if (mSendPending.load(std::memory_order_acquire))
		return true;

	mSendPending.store(true, std::memory_order_release);
	return PostSendLocked();
}

bool Session::PostSendLocked()
{
	const size_t readable = mSendBuffer->ReadableSize();
	if (readable == 0) 
	{
		mSendPending = false;
		return true;
	}

	mSendOverEx.mUID = mUID;
	mSendOverEx.mWsaBuf.len = static_cast<ULONG>(readable);
	mSendOverEx.mWsaBuf.buf = reinterpret_cast<char*>(mSendBuffer->ReadPtr());

	DWORD sent = 0;
	DWORD flags = 0;
	int ret = WSASend(
		mRemoteSock.GetSocket(),
		&mSendOverEx.mWsaBuf,
		1, &sent, flags,
		reinterpret_cast<LPWSAOVERLAPPED>(&mSendOverEx),
		nullptr);

	if (ret == SOCKET_ERROR) 
	{
		const int err = WSAGetLastError();
		if (err != WSA_IO_PENDING) 
		{
			std::cout << std::format("WSASend failed wsa:{} uid:{}\n", err, mUID);
			mSendPending = false;
			return false;
		}
	}
	return true;
}

bool Session::SendReady()
{
	const size_t storedSize = mSendBuffer->ReadableSize();
	if (storedSize == 0)
		return true;

	mSendOverEx.mUID = mUID;
	mSendOverEx.mWsaBuf.len = static_cast<ULONG>(storedSize);
	mSendOverEx.mWsaBuf.buf = reinterpret_cast<char*>(mSendBuffer->ReadPtr());

	DWORD sendBytes = 0;
	DWORD flags = 0;

	int ret = WSASend(
		mRemoteSock.GetSocket(),
		&mSendOverEx.mWsaBuf,
		1,
		&sendBytes,
		flags,
		reinterpret_cast<LPWSAOVERLAPPED>(&mSendOverEx),
		nullptr
	);

	if (ret == SOCKET_ERROR)
	{
		const int error = WSAGetLastError();
		if (error != WSA_IO_PENDING)
		{
			std::cout << std::format("SendReady: WSASend failed with error: {} id: {}\n", error, mUID);
			return false;
		}
	}

	return true;
}

void Session::SendFinish(size_t bytes)
{
	std::scoped_lock lk(mSendLock);

	size_t readable = mSendBuffer->ReadableSize();

	if (bytes > readable) 
	{
		std::cerr << std::format("OnSendComplete clip {} -> {} (uid={})\n",bytes, readable, mUID);
		bytes = readable;
	}

	mSendBuffer->MoveReadPos(bytes);

	if (mSendBuffer->ReadableSize() > 0) 
	{
		PostSendLocked();
		return;
	}

	mSendPending.store(false, std::memory_order_release);
}

bool Session::IsConnected() const 
{
	return mRemoteSock.GetSocket() != INVALID_SOCKET;
}

void Session::PrepareConnectOv() noexcept
{
	ZeroMemory(static_cast<OVERLAPPED*>(&mConnectOverEx), sizeof(OVERLAPPED));
	mConnectOverEx.mIOType = IO_TYPE::CONNECT;
	mConnectOverEx.mUID = static_cast<int>(mUID);
}