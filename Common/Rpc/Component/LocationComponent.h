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
	// 记录所有服务地址和所有玩家所在服务器
	class LocationComponent final : public Component
    {
    public:
        LocationComponent() = default;
        ~LocationComponent() = default;
    public:
		bool DelServer(int id, const std::string & name);
		void GetAllServer(std::vector<LocationUnit *> & servers);
	public:
		LocationUnit * GetServerById(int id);
		int RangeServer(const std::string & server) const;
		LocationUnit * GetOrCreateServer(int id, const std::string & name);
		bool GetAddress(int id, const std::string & listen, std::string & address);
	public:
		bool DelUnit(long long userId);
		bool DelUnit(long long userId, const std::string & server);
		void BindServer(const std::string& server, long long userId, const std::string& address); //绑定玩家转发服务器
    public:
		bool HasServer(const std::string & server) const;
        bool GetServer(const std::string & name, long long userId, std::string & address);
		bool GetServer(long long userId, std::unordered_map<std::string, std::string> & servers);
		bool GetServer(const std::string & server, const std::string & listen, std::string & address);
		bool GetServer(const std::string & server, const std::string & listen, std::vector<std::string> & servers);
	private:
		std::unordered_map<std::string, std::vector<int>> mServerNames;
		std::unordered_map<int, std::unique_ptr<LocationUnit>> mServers;
		std::unordered_map<long long, std::unique_ptr<LocationUnit>> mClients;
	};
}

#endif //APP_LOCATIONCOMPONENT_H
