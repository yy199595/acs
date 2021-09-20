#pragma once
#include<string>
#include"SocketEvent.h"
#include"PacketMapper.h"
namespace Sentry
{
	class NetSessionComponent;
	class NetProxyComponent;
	class SocketEveHandler
	{
	public:
		virtual void RunHandler(NetSessionComponent *) { };
		virtual void RunHandler(NetProxyComponent *) { };
	};

	class MainSocketSendHandler : public SocketEveHandler
	{
	public:
		MainSocketSendHandler(PacketMapper * message)
			: mSendMessage(message) { }
	public:
		void RunHandler(NetSessionComponent *) final;
	private:		
		PacketMapper * mSendMessage;
	};

	class MainSocketCloseHandler : public SocketEveHandler
	{
	public:
		MainSocketCloseHandler(const std::string & address)
			:mAddress(address) { }
	public:
		void RunHandler(NetSessionComponent *) final;
	private:
		const std::string mAddress;
	};

	class MainSocketConnectHandler : public SocketEveHandler
	{
	public:
		MainSocketConnectHandler(const std::string & address, const std::string & name)
			: mAddress(address), mName(name) {}
	public:
		void RunHandler(NetSessionComponent *) final;
	private:
		const std::string mName;
		const std::string mAddress;
	};
}

namespace Sentry
{
	class NetSocketConnectHandler : public SocketEveHandler
	{
	public:
		NetSocketConnectHandler(const std::string & address, bool isSuc)
			: mIsSuccessful(isSuc), mAddress(address) { }
	public:
		void RunHandler(NetProxyComponent * component);
	private:
		const bool mIsSuccessful;
		const std::string mAddress;
	};

	class NetNewSocketConnectHandler : public SocketEveHandler
	{
	public:
		NetNewSocketConnectHandler(const std::string & address)
			:mAddress(address) { }
	public:
		void RunHandler(NetProxyComponent * component);
	private:
		const std::string mAddress;
	};

	class NetErrorHandler : public SocketEveHandler
	{
	public:
		NetErrorHandler(const std::string & address)
			: mAddress(address) { }
	public:
		void RunHandler(NetProxyComponent * component);
	private:
		const std::string mAddress;
	};

	class NetReceiveNewMessageHandler : public SocketEveHandler
	{
	public:
		NetReceiveNewMessageHandler(PacketMapper * message)
			: mRecvMessage(message) { }
	public:
		void RunHandler(NetProxyComponent * component);
	private:		
		PacketMapper * mRecvMessage;
	};
}