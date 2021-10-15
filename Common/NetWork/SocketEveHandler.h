#pragma once
#include<string>
#include"SocketEvent.h"
#include<Define/CommonTypeDef.h>
namespace Sentry
{
	class TcpNetSessionComponent;
	class TcpNetProxyComponent;
	class SocketEveHandler
	{
	public:
		virtual void RunHandler(TcpNetSessionComponent *) { };
		virtual void RunHandler(TcpNetProxyComponent *) { };
	};

	class MainSocketSendHandler : public SocketEveHandler
    {
    public:
        MainSocketSendHandler(const std::string &address, SharedMessage message) :
                mAddress(address), mMessage(message)
        {}

        MainSocketSendHandler(const std::string &address, const char *msg, size_t size)
                : mAddress(address), mMessage(make_shared<std::string>(msg, size))
        {}

    public:
        void RunHandler(TcpNetSessionComponent *) final;

    private:
        std::string mAddress;
        SharedMessage mMessage;
    };

	class MainSocketCloseHandler : public SocketEveHandler
	{
	public:
		MainSocketCloseHandler(const std::string & address)
			:mAddress(address) { }
	public:
		void RunHandler(TcpNetSessionComponent *) final;
	private:
		const std::string mAddress;
	};

	class MainSocketConnectHandler : public SocketEveHandler
	{
	public:
		MainSocketConnectHandler(const std::string & address, const std::string & name)
			: mAddress(address), mName(name) {}
	public:
		void RunHandler(TcpNetSessionComponent *) final;
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
        NetSocketConnectHandler(const std::string &address, bool isSuc)
                : mIsSuccessful(isSuc), mAddress(address)
        {}

    public:
        void RunHandler(TcpNetProxyComponent *component);

    private:
        const bool mIsSuccessful;
        const std::string mAddress;
    };

    class NetNewSocketConnectHandler : public SocketEveHandler
    {
    public:
        NetNewSocketConnectHandler(const std::string &address)
                : mAddress(address)
        {}

    public:
        void RunHandler(TcpNetProxyComponent *component);

    private:
        const std::string mAddress;
    };

    class NetErrorHandler : public SocketEveHandler
    {
    public:
        NetErrorHandler(const std::string &address)
                : mAddress(address)
        {}

    public:
        void RunHandler(TcpNetProxyComponent *component);

    private:
        const std::string mAddress;
    };

    class NetReceiveNewMessageHandler : public SocketEveHandler
    {
    public:
        NetReceiveNewMessageHandler(const std::string &address, const char * msg, size_t size)
                : mAddress(address), mMessage(make_shared<std::string>(msg, size))
        {}

        ~NetReceiveNewMessageHandler();

    public:
        void RunHandler(TcpNetProxyComponent *component);

    private:
        std::string mAddress;
        SharedMessage mMessage;
    };
}