//
// Created by zmhy0073 on 2022/8/12.
//

#ifndef APP_LOCATIONCOMPONENT_H
#define APP_LOCATIONCOMPONENT_H
#include<vector>
#include<string>
#include<unordered_set>
#include<unordered_map>
#include"Entity/Unit/LocationUnit.h"
#include"Entity/Component/Component.h"

namespace Tendo
{
	class ServerData
	{
	public:
		ServerData(const std::string & name);
		bool operator == (const ServerData & data) const {
			return this->mRpc == data.mRpc;
		}
		bool operator == (const std::string & rpc) const {
			return this->mRpc == rpc;
		}
	public:
		const std::string & Name() const { return this->mName; }
		const std::string & RpcAddress() const { return this->mRpc; }
		bool Get(const std::string & listen, std::string & address) const;
		bool Add(const std::string & listen, const std::string & address);
	private:
		std::string mRpc;
		std::string mName;
		std::unordered_map<std::string, std::string> mListens;
	};
}
//namespace std
//{
//	template<> struct std::hash<Tendo::ServerData>
//	{
//		size_t operator() (const Tendo::ServerData & data) const{
//			std::hash<std::string> hash;
//			return hash(data.RpcAddress());
//		}
//	};
//}


namespace Tendo
{
	// 记录所有服务地址和所有玩家所在服务器
	class LocationComponent final : public Component
    {
    public:
        LocationComponent() = default;
        ~LocationComponent() = default;
    public:
        bool AddServer(const ServerData & serverData);
		bool DelServer(const std::string & server, const std::string & rpc);
	public:
		bool DelUnit(long long userId);
		bool DelUnit(const std::string & server, long long userId);
		void BindServer(const std::string& server, long long userId, const std::string& address); //绑定玩家转发服务器
    public:
		bool HasServer(const std::string & server) const;
        bool GetServer(const std::string & name, long long userId, std::string & address);
		bool GetServer(long long userId, std::unordered_map<std::string, std::string> & servers);
		bool GetServer(const std::string & server, std::string & address, const std::string & listen = "rpc");
		bool GetServer(const std::string & server, std::vector<std::string> & servers, const std::string & listen = "rpc");
	private:
		//std::unordered_map<int, std::unique_ptr<LocationUnit>> mServers;
		std::unordered_map<long long, std::unique_ptr<LocationUnit>> mClients;
		std::unordered_map<std::string, std::vector<ServerData>> mServers;
	};
}

#endif //APP_LOCATIONCOMPONENT_H
