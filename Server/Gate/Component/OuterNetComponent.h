//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_GATECLIENTCOMPONENT_H
#define GAMEKEEPER_GATECLIENTCOMPONENT_H

#include"Client/Message.h"
#include"Component/Component.h"
#include"Component/TcpListenerComponent.h"
namespace Sentry
{

	class OuterNetClient;
	class OuterNetComponent : public TcpListenerComponent, public IRpc<Rpc::Data>
	{
	 public:
		OuterNetComponent() = default;
		~OuterNetComponent() final = default;
	 public:
		void StartClose(const std::string & address) final;
		void OnCloseSocket(const std::string & address, XCode code) final;
        void OnMessage(const std::string &address, std::shared_ptr<Rpc::Data> message) final;
    public:
        bool SendData(std::shared_ptr<Rpc::Data> message);
		OuterNetClient* GetGateClient(const std::string& address);
        bool SendData(long long userId, std::shared_ptr<Rpc::Data> message);
        bool SendData(const std::string & address, std::shared_ptr<Rpc::Data> message);
	 public:
		bool Awake() final;
		bool LateAwake() final;
	 private:
        void OnStopListen() final;
        bool OnListen(std::shared_ptr<SocketProxy> socket) final;
        bool OnAuth(const std::string & address, std::shared_ptr<Rpc::Data> message);
        bool OnRequest(const std::string & address, std::shared_ptr<Rpc::Data> message);
    private:
		class TimerComponent* mTimerComponent;
        class NetThreadComponent * mNetComponent;
        class LocationComponent * mLocationComponent;
        class OuterNetMessageComponent* mOuterMessageComponent;
        std::queue<std::shared_ptr<OuterNetClient>> mClientPools;
		std::unordered_map<std::string, std::shared_ptr<OuterNetClient>> mGateClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
