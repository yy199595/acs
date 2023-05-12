//
// Created by zmhy0073 on 2022/8/12.
//

#ifndef APP_LOCATIONCOMPONENT_H
#define APP_LOCATIONCOMPONENT_H
#include<vector>
#include<string>
#include<unordered_set>
#include<unordered_map>
#include"Entity/Unit/ServerUnit.h"
#include"Entity/Component/Component.h"

namespace Tendo
{
	// 记录所有服务地址和所有玩家所在服务器
	class LocationComponent final : public Component
    {
    public:
        LocationComponent() = default;
        ~LocationComponent() = default;
    public:
		bool DelServer(int id);
		void GetAllServer(std::vector<ServerUnit *> & servers);
	public:
		ServerUnit * GetServerById(int id);
		ClientUnit * GetClientById(long long id);
		int RangeServer(const std::string & server) const;
		ServerUnit * GetOrCreateServer(int id, const std::string & name);
	public:
		bool DelUnit(long long userId);
		bool DelUnit(const std::string & server, long long userId);
		void BindServer(const std::string& server, long long userId, int serverId); //绑定玩家转发服务器
    public:
		bool HasServer(const std::string & server) const;
		bool GetServerAddress(int id, const std::string & listen, std::string & address);
		bool GetServerAddress(long long userId, const std::string & server, const std::string & listen, std::string & address);
	private:
		std::unordered_map<std::string, std::vector<int>> mServerNames;
		std::unordered_map<int, std::unique_ptr<ServerUnit>> mServers;
		std::unordered_map<long long, std::unique_ptr<ClientUnit>> mClients;
	};
}

#endif //APP_LOCATIONCOMPONENT_H
