#pragma once
#include <Define/CommonTypeDef.h>
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
        virtual bool OnRequestMessage(const std::string & address, SharedMessage message) = 0; //注意message内存释放
    };

    class IResponseMessageHandler
    {
    public:
        virtual bool OnResponseMessage(const std::string & address, SharedMessage message) = 0; //注意message内存释放
    };
}

namespace Sentry
{
    class TcpClientSession;
    class ISessionHandler
    {
    public:
        virtual void OnSessionError(TcpClientSession *session) = 0;
        virtual void OnConnectComplete(TcpClientSession *session, bool isSuc) = 0;
        virtual bool OnRecvMessage(TcpClientSession *session, const char *message, const size_t size) = 0;
    };
}