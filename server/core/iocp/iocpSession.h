#pragma once
#include "pch.h"
#include "iocpBuffer.h"
#include "iocp\iocpSocket.h"
enum class IO_TYPE
{
	NONE,
	RECV,
	SEND,
	ACCEPT
};

struct OverlappedIoEx : public OVERLAPPED
{
	WSABUF mWsaBuf;
	int mUID;
	IO_TYPE mIOType;

	OverlappedIoEx(IO_TYPE type = IO_TYPE::NONE) :OVERLAPPED{}, mWsaBuf{}, mUID(-1), mIOType(type) {}
};

class IocpSession
{
public:
	IocpSession();
	~IocpSession();

	void			Init();
	void			DisconnectFinish();

	bool			AcceptReady(const SOCKET& listenSock, const int uID);
	bool			AcceptFinish(const SOCKET& listenSocket);

	bool			RecvPacket(unsigned long ioSize);
	bool			RecvReady();

	bool			SendPacket(const char* data, unsigned long long packetSize);
	bool			SendReady();

	void			SetUniqueId(int id) { mUID = id; }

	unsigned int	GetUniqueId() const { return mUID; }
	const SOCKET&	GetRemoteSocket() const { return mRemoteSock.GetSocket(); }
	IocpBuffer*		GetRecvBuffer() const { return mRecvBuffer.get(); }
	IocpBuffer*		GetSendBuffer() const { return mSendBuffer.get(); }
	char*			GetRecvOverlappedBuffer() const;

	bool			InitRemoteSocket() { mRemoteSock.Init(); }
	bool			IsConnected() const;
private:
	IocpSocket					mRemoteSock;
	OverlappedIoEx				mRecvOverEx;
	OverlappedIoEx				mSendOverEx;
	OverlappedIoEx              mAcceptOverEx;

	unsigned int				mUID = -1;
	char						mAcceptBuf[64]{};

private:
	std::unique_ptr<IocpBuffer> mRecvBuffer{ std::make_unique<IocpBuffer>(RECV_BUFFER_MAX_SIZE) };
	std::unique_ptr<IocpBuffer> mSendBuffer{ std::make_unique<IocpBuffer>(SEND_BUFFER_MAX_SIZE) };
};