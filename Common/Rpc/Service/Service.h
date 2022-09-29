#pragma once

#include<memory>
#include"ServiceHost.h"
#include"Client/Message.h"
#include"Message/c2s.pb.h"
#include"Json/JsonWriter.h"
#include"Config/ServiceConfig.h"
#include"Component/Component.h"

using namespace std;

namespace Sentry
{
	class ServiceMethod;
	class InnerNetClient;
    class Service : public Component, public ILuaRegister, public ServiceHost, public IServiceBase
	{
	 public:
		Service();
		virtual ~Service() override = default;
	 public:
		XCode Send(const std::string& func, const Message & message);
		XCode Send(long long userId, const std::string& func, const Message & message);
		XCode Send(const std::string & address, const std::string& func, const Message & message);
	 public:
		XCode Call(const std::string& address, const std::string& func);
		XCode Call(const std::string& address, const std::string& func, const Message& message);
		XCode Call(const std::string& address, const std::string& func, std::shared_ptr<Message> response);
		XCode Call(const std::string& address, const std::string& func, const Message& message, std::shared_ptr<Message> response);
	 public:
		XCode Call(long long userId, const std::string& func);
		XCode Call(long long userId, const std::string& func, const Message& message);
		XCode Call(long long userId, const std::string& func, std::shared_ptr<Message> response);
        XCode Call(long long userId, const std::string& func, const Message& message, std::shared_ptr<Message> response);
	public:
        bool IsStartComplete() final{return this->GetHostSize() > 0; };
        const RpcServiceConfig & GetServiceConfig() { return *this->mConfig; }
        const std::string & GetLocalHost() final { return this->mLocalAddress; }
        const std::string & GetServiceName() final { return this->GetName(); }
    public:
        bool StartSend(const std::string & address, const std::string & func, long long userId, const Message * message);
        std::shared_ptr<Rpc::Data> StartCall(const std::string & address, const std::string & func, long long userId, const Message * message);
	 protected:
		bool LateAwake() override;
		bool LoadConfig(const rapidjson::Value & json);
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) override;

    public:
        virtual XCode Invoke(const std::string & method, std::shared_ptr<Rpc::Data> message) = 0;
	private:
        RpcServiceConfig * mConfig;
        std::string mLocalAddress;
        std::vector<std::string> mServiceHosts;
        class InnerNetComponent* mClientComponent;
        class InnerNetMessageComponent* mMessageComponent;
    };
}