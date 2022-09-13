//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_GATECLIENTCOMPONENT_H
#define GAMEKEEPER_GATECLIENTCOMPONENT_H
#include"Component/Component.h"
#include"Network/Rpc.h"
#include"Network/Listener/TcpServerListener.h"
namespace Sentry
{
	class OuterNetClient;
	class OuterNetComponent : public Component, public TcpServerListener,
                              public IRpc<Tcp::BinMessage>
	{
	 public:
		OuterNetComponent() = default;
		~OuterNetComponent() final = default;
	 public:
		void StartClose(const std::string & address) final;
		void OnCloseSocket(const std::string & address, XCode code) final;
        void OnMessage(const std::string &address, std::shared_ptr<Tcp::BinMessage> message) final;
    public:
		bool AddNewUser(const std::string & address, long long userId);
		bool GetUserId(const std::string & address, long long & userId);
		bool GetUserAddress(long long userId, std::string & address);
		std::shared_ptr<OuterNetClient> GetGateClient(const std::string & address);
	 public:
		void SendToAllClient(std::shared_ptr<c2s::rpc::call> message);
		bool SendToClient(const std::string & address, std::shared_ptr<c2s::rpc::call> message);
		bool SendToClient(const std::string & address, std::shared_ptr<c2s::rpc::response> message);
	 public:
		void Awake() final;
		bool LateAwake() final;

		bool OnListen(std::shared_ptr<SocketProxy> socket) final;
	 private:
        bool StartInComplete() final { return false; }
		bool OnRequest(const std::string& address, const Tcp::BinMessage & message);
    private:
		class TimerComponent* mTimerComponent;
        class NetThreadComponent * mNetComponent;
        class OuterNetMessageComponent* mOuterMessageComponent;
        std::queue<std::shared_ptr<OuterNetClient>> mClientPools;
        std::unordered_map<std::string, long long> mUserAddressMap;
        std::unordered_map<long long, std::string> mClientAddressMap;
		std::unordered_map<std::string, std::shared_ptr<OuterNetClient>> mGateClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
