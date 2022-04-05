#pragma once
#include<Thread/TaskThread.h>
#include<Protocol/com.pb.h>
#include"Protocol/c2s.pb.h"
#include<Pool/StringPool.h>
#include"XCode/XCode.h"
#include"Json/JsonReader.h"
namespace Sentry
{
	class Component;
	class IStart
	{
	 public:
		virtual void OnStart() = 0;
	};

	class IComplete
	{
	 public:
		virtual void OnComplete() = 0;
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
		virtual void GetRefreshTime(int& hour, int& min)
		{
			hour = 0;
			min = 0;
		}
	};

	class IRemoteService
	{
	 public:
		virtual void OnServiceExit(const std::string & address) = 0;
		virtual void OnServiceJoin(const std::string & name, const std::string & address) = 0;
	};

	template<typename T>
	class ILocalService
	{
	 public:
		virtual void OnAddService(T * component) = 0;
		virtual void OnDelService(T * component) = 0;
	};

	template<typename T1, typename T2>
	class IRpc
	{
	 public:
		virtual void StartClose(long long id) = 0;
		virtual void OnRequest(std::shared_ptr<T1> t1) = 0;
		virtual void OnResponse(std::shared_ptr<T2> t2) = 0;
		virtual void OnCloseSocket(long long id, XCode code) = 0;
		virtual void OnConnectAfter(long long id, XCode code) = 0;
		virtual void OnSendFailure(long long id, std::shared_ptr<google::protobuf::Message> message)
		{
		}
	};

	template<typename T1, typename T2>
	class IProtoRpc
	{
	 public:
		virtual XCode OnRequest(std::shared_ptr<T1> request) = 0;
		virtual XCode OnResponse(std::shared_ptr<T2> response) = 0;
	};

	template<typename T1, typename T2>
	class IClientRpc
	{
	 public:
		virtual XCode OnRequest(std::shared_ptr<T1> request) = 0;
		virtual XCode OnResponse(long long sockId, const std::shared_ptr<T2> response) = 0;
	};

	class IProtoResponse
	{
	 public:
		virtual bool OnResponse(const com::Rpc_Response* message) = 0;
	};

	class IJsonRequest
	{
	 public:
		virtual bool OnRequest(const Json::Reader* message) = 0;
	};

	class IJsonResponse
	{
	 public:
		virtual bool OnResponse(const Json::Reader* message) = 0;
	};

	class SocketProxy;
	class ISocketListen
	{
	 public:
		virtual void OnListen(std::shared_ptr<SocketProxy> socket) = 0;
	};
}