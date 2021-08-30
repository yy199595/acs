#pragma once
#include<string>
#include"SocketEvent.h"
#include"PacketMapper.h"
namespace Sentry
{
	class SceneSessionComponent;
	class SceneNetProxyComponent;
	class SocketEveHandler
	{
	public:
		virtual void RunHandler(SceneSessionComponent *) { };
		virtual void RunHandler(SceneNetProxyComponent *) { };
	};

	class MainSocketSendHandler : public SocketEveHandler
	{
	public:
		MainSocketSendHandler(const std::string & address, PacketMapper * message)
			: mAddress(address), mSendMessage(message) { }
	public:
		void RunHandler(SceneSessionComponent *) final;
	private:		
		const std::string mAddress;
		PacketMapper * mSendMessage;
	};

	class MainSocketCloseHandler : public SocketEveHandler
	{
	public:
		MainSocketCloseHandler(const std::string & address)
			:mAddress(address) { }
	public:
		void RunHandler(SceneSessionComponent *) final;
	private:
		const std::string mAddress;
	};

	class MainSocketConnectHandler : public SocketEveHandler
	{
	public:
		MainSocketConnectHandler(const std::string & address, const std::string & name)
			: mAddress(address), mName(name) {}
	public:
		void RunHandler(SceneSessionComponent *) final;
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
		void RunHandler(SceneNetProxyComponent * component);
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
		void RunHandler(SceneNetProxyComponent * component);
	private:
		const std::string mAddress;
	};

	class NetErrorHandler : public SocketEveHandler
	{
	public:
		NetErrorHandler(const std::string & address)
			: mAddress(address) { }
	public:
		void RunHandler(SceneNetProxyComponent * component);
	private:
		const std::string mAddress;
	};

	class NetReceiveNewMessageHandler : public SocketEveHandler
	{
	public:
		NetReceiveNewMessageHandler(const std::string & address, PacketMapper * message)
			:mAddress(address), mRecvMessage(message) { }
	public:
		void RunHandler(SceneNetProxyComponent * component);
	private:
		const std::string mAddress;
		PacketMapper * mRecvMessage;
	};
}