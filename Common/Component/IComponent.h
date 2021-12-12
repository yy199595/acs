#pragma once
#include<Thread/TaskThread.h>
#include<Protocol/com.pb.h>
#include"Protocol/c2s.pb.h"
#include<Pool/StringPool.h>
#include"XCode/XCode.h"
namespace GameKeeper
{
    class IStart
    {
    public:
        virtual void OnStart() = 0;
    };
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
        virtual void OnLoadData() = 0;
    };

    class ILoadConfig
    {
    public:
        virtual bool OnLoadConfig() = 0;
    };

    class IZeroRefresh
    {
    public:
        virtual void OnZeroRefresh() = 0;
        virtual void GetRefreshTime(int & hour, int & min)
        {
            hour = 0; min = 0;
        }
    };

	template<typename T1, typename T2>
	class IRpc
	{
	public:
        virtual void OnRequest(T1 * t1) = 0;
        virtual void OnResponse(T2 * t2) = 0;
        virtual void StartClose(long long id) = 0;
        virtual void OnCloseSocket(long long id, XCode code) = 0;
        virtual void OnConnectAfter(long long id, XCode code) = 0;
    };

    template<typename T1, typename T2>
    class IProtoRpc
    {
    public:
        virtual bool OnRequest(const T1 * request) = 0;
        virtual bool OnResponse(const T2 * response) = 0;
    };

    template<typename T1, typename T2>
    class IClientProtoRpc
    {
    public:
        virtual XCode OnRequest(const T1 * request) = 0;
        virtual XCode OnResponse(long long sockId, const T2 * response) = 0;
    };

    class IProtoResponse
    {
    public:
        virtual bool OnResponse(const com::Rpc_Response * message) = 0;
    };
	 
    class IJsonRequest
    {
    public:
        virtual bool OnRequest(const class RapidJsonReader * message) = 0;
    };

    class IJsonResponse
    {
    public:
        virtual bool OnResponse(const class RapidJsonReader * message) = 0;
    };

    class INodeProxyRefresh
    {
    public:
        virtual void OnAddProxyNode(class RpcNodeProxy * node) = 0; //添加服务节点
        virtual void OnDelProxyNode(class RpcNodeProxy * node) = 0; //删除服务节点
    };


	class SocketProxy;
	class ISocketListen
	{
	public:
		virtual void OnListen(SocketProxy * socket) = 0;
	};
}