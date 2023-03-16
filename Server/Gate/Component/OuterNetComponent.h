//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_GATECLIENTCOMPONENT_H
#define GAMEKEEPER_GATECLIENTCOMPONENT_H
#include<queue>
#include<unordered_map>
#include<unordered_set>
#include"Component/TcpListenerComponent.h"

namespace Rpc
{
    class Packet;
}
namespace Sentry
{

	class OuterNetClient;
    class OuterNetComponent : public TcpListenerComponent, public IRpc<Rpc::Packet>,
							  public IComplete, public IServerRecord, public IFrameUpdate
	{
	 public:
		OuterNetComponent() = default;
		~OuterNetComponent() final = default;
	 public:
		void StartClose(const std::string & address) final;
		void OnMessage(std::shared_ptr<Rpc::Packet> message) final;
		void OnCloseSocket(const std::string & address, int code) final;
    public:
		bool MakeToken(long long id, std::string & token);
		size_t Broadcast(const std::shared_ptr<Rpc::Packet> & message);
		bool GetUserId(const std::string & address, long long & userId) const;
		bool Send(long long userId, const std::shared_ptr<Rpc::Packet> & message);
		bool Send(const std::string & address, const std::shared_ptr<Rpc::Packet> & message);
	private:
		bool Awake() final;
		bool LateAwake() final;
        void OnDestroy() final;
        void OnClusterComplete() final;
		void OnFrameUpdate(float t) final;
	 private:
        void OnRecord(Json::Writer & document) final;
        void OnListen(std::shared_ptr<SocketProxy> socket) final;
        bool OnAuth(std::shared_ptr<Rpc::Packet> message);
	 private:
		int mMaxHandlerCount;
		unsigned int mSumCount;
		unsigned int mWaitCount;
		std::queue<std::shared_ptr<Rpc::Packet>> mMessages;
        class OuterNetMessageComponent* mOuterMessageComponent;
        std::queue<std::shared_ptr<OuterNetClient>> mClientPools;
		std::unordered_map<std::string, long long> mClientTokens;
		std::unordered_map<std::string, long long> mAddressUserMap; //验证过的客户端
		std::unordered_map<long long, std::string> mUserAddressMap; //验证过的客户端
		std::unordered_map<std::string, std::shared_ptr<OuterNetClient>> mGateClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
