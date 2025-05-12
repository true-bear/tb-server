#pragma once
#include "pch.h"
#include "clientSession.h"
#include "logger.h"


ClientSession::ClientSession()
{
	ZeroMemory(&mRecvOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mSendOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mAcceptOverEx, sizeof(OverlappedIoEx));

	mRecvOverEx.mIOType = IO_TYPE::RECV;
	mSendOverEx.mIOType = IO_TYPE::SEND;
	mAcceptOverEx.mIOType = IO_TYPE::ACCEPT;
}

ClientSession::~ClientSession() = default;

void ClientSession::Init()
{
	ZeroMemory(&mRecvOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mSendOverEx, sizeof(OverlappedIoEx));
	ZeroMemory(&mAcceptOverEx, sizeof(OverlappedIoEx));

	mRecvOverEx.mIOType = IO_TYPE::RECV;
	mSendOverEx.mIOType = IO_TYPE::SEND;
	mAcceptOverEx.mIOType = IO_TYPE::ACCEPT;

	mRecvBuffer->Clear();
	mSendBuffer->Clear();

	ZeroMemory(mAcceptBuf, sizeof(mAcceptBuf));
}


void ClientSession::DisconnectFinish()
{
	if (auto socket = mRemoteSock.GetSocket(); socket != INVALID_SOCKET)
	{
		shutdown(socket, SD_BOTH);
		closesocket(socket);
	}
}

bool ClientSession::AcceptReady(const SOCKET& listenSock, const int uID)
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

bool ClientSession::AcceptFinish(const SOCKET& listenSocket)
{
	auto remoteSocket = mRemoteSock.GetSocket();

	if (setsockopt(remoteSocket,
		SOL_SOCKET,
		SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&listenSocket,
		sizeof(listenSocket)) == SOCKET_ERROR)
	{
		LOG_ERR("AcceptFinish", "err:{} \n", WSAGetLastError());
		return false;
	}

	sockaddr_in clientAddr;
	if (!mRemoteSock.GetPeerName(clientAddr))
	{
		LOG_ERR("AcceptFinish", "Failed to get peer name");
		return false;
	}

	int flag = 1;
	if (!mRemoteSock.SetOption(IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)))
	{
		LOG_WARN("AcceptFinish", "Failed to set TCP_NODELAY option");
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
	//LOG_INFO("AcceptFinish", "Client connected from: {}:{}", clientIP, ntohs(clientAddr.sin_port));

	return true;
}

bool ClientSession::RecvReady()
{
	char* buf = mRecvBuffer->GetWritePtr();
	unsigned long len = static_cast<unsigned long>(mRecvBuffer->GetContinuousWriteSize());

	mRecvOverEx.mUID = mUID;
	mRecvOverEx.mWsaBuf = { len, buf };

	unsigned long recvBytes = 0;
	unsigned long flags = 0;

	int ret = WSARecv(
		mRemoteSock.GetSocket(),
		&mRecvOverEx.mWsaBuf,
		1,
		&recvBytes,
		&flags,
		reinterpret_cast<LPWSAOVERLAPPED>(&mRecvOverEx),
		NULL
	);

	if (ret == SOCKET_ERROR)
	{
		if (int error = WSAGetLastError(); error != WSA_IO_PENDING)
			return false;
	}

	return true;
}

bool ClientSession::RecvPacket(unsigned long ioSize)
{
	if (ioSize == 0 || ioSize > RECV_BUFFER_MAX_SIZE)
	{
		LOG_ERR("RecvPacket", "iosize :{} id:{}", ioSize, mUID);
		return false;
	}

	mRecvBuffer->MoveWritePos(ioSize);
	return true;
}

char* ClientSession::GetRecvOverlappedBuffer() const
{
	return mRecvOverEx.mWsaBuf.buf;
}

bool ClientSession::SendPacket(const char* data, unsigned long long packetSize)
{
	uint16_t sizeHeader = static_cast<uint16_t>(packetSize);
	uint16_t sizeHeaderBE = htons(sizeHeader);

	if (!mSendBuffer->Write((char*)&sizeHeaderBE, sizeof(sizeHeaderBE)) ||
		!mSendBuffer->Write(data, packetSize))
	{
		LOG_ERR("SendPacket", "send buffer header size:{} id:{}", sizeof(sizeHeaderBE), mUID);
		return false;
	}

	return SendReady();
}

bool ClientSession::SendReady()
{
	size_t storedSize = mSendBuffer->GetStoredSize();

	if (storedSize <= 0)
		return true;

	std::vector<char> sendData(storedSize);
	if (!mSendBuffer->Read(sendData.data(), storedSize))
	{
		LOG_ERR("SendReady", "send buffer read size:{} id:{}", storedSize, mUID);
		return false;
	}


	mSendOverEx.mUID = mUID;
	mSendOverEx.mWsaBuf = { (ULONG)storedSize , sendData.data() };

	unsigned long sendBytes = 0;
	unsigned long flags = 0;

	int ret = WSASend(
		mRemoteSock.GetSocket(),
		&mSendOverEx.mWsaBuf,
		1,
		&sendBytes,
		flags,
		reinterpret_cast<LPWSAOVERLAPPED>(&mSendOverEx),
		NULL
	);

	if (ret == SOCKET_ERROR)
	{
		if (int error = WSAGetLastError(); error != WSA_IO_PENDING)
		{
			LOG_ERR("SendReady", "WSASend err:{} id:{}", error, mUID);
			return false;
		}
	}

	return true;
}

bool ClientSession::IsConnected() const {
	return mRemoteSock.GetSocket() != INVALID_SOCKET;
}