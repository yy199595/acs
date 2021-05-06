#pragma once
#include<asio.hpp>
#include<queue>
#define ASIO_BUFFER_SIZE 2048
typedef std::shared_ptr<asio::ip::tcp::socket> SharedSocketPtr;
class NetWorkClient
{
public:
	NetWorkClient(asio::io_context & io, const std::string ip, const unsigned short port);
public:
	void StartConnect();
	void StartRecvMsg();
	void SendPackage(const std::string & message);
	bool TryGetNetMessage(std::string & message);
private:
	void ReadMessageBody(size_t size);
public:
	void CloseSocket();
	bool IsConnect() { return this->mIsConnect; }
	std::string GetPacketBody(size_t size);
private:
	bool mIsConnect;
	std::string mIp;
	unsigned short mPort;
	std::string mAddress;
	SharedSocketPtr mTcpSocket;
	asio::io_context & mAsioContext;
	std::queue<std::string> mRecvMessageQueue;
	char mRecvMessageBuffer[ASIO_BUFFER_SIZE];
	char mSendMessageBuffer[ASIO_BUFFER_SIZE];
};