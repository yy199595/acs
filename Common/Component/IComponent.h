#pragma once
#include <Define/CommonTypeDef.h>
#include<NetWork/SessionBase.h>
namespace Sentry
{
	class IFrameUpdate
	{
	public:
		virtual void OnFrameUpdate(float t) = 0;
	};

	class ISystemUpdate
	{
	public:
		virtual void OnSystemUpdate() = 0;
	};

	class ISecondUpdate
	{
	public:
		virtual void OnSecondUpdate() = 0;
	};

	class ILastFrameUpdate
	{
	public:
		virtual void OnLastFrameUpdate() = 0;
	};

	class ITcpContextUpdate
	{
	public:
		virtual void OnTcpContextUpdate(AsioContext & io) = 0;
	};

    class IHttpContextUpdate
    {
    public:
        virtual void OnHttpContextUpdate(AsioContext & io) = 0;
    };

	class IHotfix
	{
	public:
		virtual void OnHotFix() = 0;
	};

    class ILoadData
    {
    public:
        virtual void OnLodaData() = 0;
    };

    class IZeroRefresh
    {
    public:
        virtual void OnZeroRefresh() = 0;
    };

    class IRequestMessageHandler
    {
    public:
        virtual bool OnRequestMessage(const std::string & address, SharedMessage message) = 0;
    };

    class IResponseMessageHandler
    {
    public:
        virtual bool OnResponseMessage(const std::string & address, SharedMessage message) = 0;
    };
}

namespace Sentry
{
	class SessionBase;
	class NetWorkThread;
    class TcpClientSession;
	
    class ISocketHandler
    {
    public:
		
		virtual void OnClose(SessionBase * socket) = 0;
		virtual SessionBase * CreateSocket(AsioContext & io) = 0;
		virtual void OnSessionErr(SessionBase * session, const asio::error_code & err) = 0;
		virtual void OnConnectRemote(SessionBase * session, const asio::error_code & err) = 0;
		virtual void OnListenConnect(NetWorkThread * netTask, SessionBase * session) = 0;		
		virtual void OnReceiveNewMessage(SessionBase * session, SharedMessage message) = 0;
		
	public:
		virtual NetWorkThread * GetNetThread() { return mNetThread; };
		void SetNetThread(NetWorkThread * t) { this->mNetThread = t; };
	private:
		NetWorkThread * mNetThread;
    };
}