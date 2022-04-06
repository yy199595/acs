//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALRPCSERVICE_H
#define SERVER_LOCALRPCSERVICE_H
#include"RpcServiceBase.h"
#include"Method/SubMethod.h"
#include"Method/ServiceMethod.h"

namespace Sentry
{
	class LocalRpcService : public RpcServiceBase
	{
	public:
		LocalRpcService() = default;
	public:
		void AddNodeAddress(const std::string &address) final;
		void DelNodeAddress(const std::string &address) final;
	public:
		bool AddMethod(std::shared_ptr<ServiceMethod> method);
		std::shared_ptr<com::Rpc_Response> Invoke(const std::string& method, std::shared_ptr<com::Rpc_Request> request);

		long long GetCurSocketId() const
		{
			return this->mCurSocketId;
		}

		void SetCurSocketId(long long socketId)
		{
			this->mCurSocketId = socketId;
		}

	protected:

		bool Awake() final;

		template<typename T>
		bool Bind(std::string name, ServiceMethodType1<T> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod1<T>>(name, (T*)this, func));
		}

		template<typename T>
		bool Bind(std::string name, ServiceMethodType11<T> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod1<T>>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType2<T, T1> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType22<T, T1> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this, func));
		}

		template<typename T, typename T1, typename T2>
		bool Bind(std::string name, ServiceMethodType3<T, T1, T2> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this, func));
		}

		template<typename T, typename T1, typename T2>
		bool Bind(std::string name, ServiceMethodType33<T, T1, T2> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType4<T, T1> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType44<T, T1> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this, func));
		}

	private:
		std::shared_ptr<ServiceMethod> GetMethod(const std::string& name);

	private:
		long long mCurSocketId;
		std::shared_ptr<ServiceNode> mLocalNode;
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mMethodMap;
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mLuaMethodMap;
		std::unordered_map<std::string, std::shared_ptr<ServiceNode>> mRemoteNodes;
	};
#define BIND_RPC_FUNCTION(func) LOG_CHECK_RET_FALSE(this->Bind(GetFunctionName(#func), &func))
}
#endif //SERVER_LOCALRPCSERVICE_H
