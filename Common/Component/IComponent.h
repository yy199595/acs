#pragma once
#include<Thread/TaskThread.h>
#include<Protocol/com.pb.h>
#include<Pool/StringPool.h>
namespace GameKeeper
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

    class IProtoRequest
    {
    public:
        virtual bool OnRequest(const com::Rpc_Request & message) = 0;
    };

    class IProtoResponse
    {
    public:
        virtual bool OnResponse(const com::Rpc_Response & message) = 0;
    };

    class IJsonRequest
    {
    public:
        virtual bool OnRequest() = 0;
    };

    class IJsonResponse
    {
    public:
        virtual bool OnResponse() = 0;
    };


	class SocketProxy;
	class ISocketListen
	{
	public:
		virtual void OnListen(SocketProxy * socket) = 0;
	};
}