//
// Created by zmhy0073 on 2022/8/12.
//

#ifndef APP_LOCATIONCOMPONENT_H
#define APP_LOCATIONCOMPONENT_H
#include<vector>
#include<string>
#include<unordered_map>
#include"Entity/Unit/LocationUnit.h"
#include"Entity/Component/Component.h"

namespace Tendo
{
	class NodeMgrComponent final : public Component
    {
    public:
        NodeMgrComponent() = default;
        ~NodeMgrComponent() = default;
    public:
        bool AddServer(int id, const std::string & server, const std::string & name, const std::string & address);
	 public:
		bool DelServer(int id);
		bool DelUnit(long long id);
        bool AddRpcServer(const std::string& server, long long id, const std::string& address);
    public:
		bool HasServer(const std::string & server) const;
        bool GetServer(const std::string & name, long long userId, std::string & address);
		bool GetServer(long long userId, std::unordered_map<std::string, std::string> & servers);
		bool GetServer(const std::string & server, std::string & address, const char * listen = nullptr);
		bool GetServer(const std::string & server, std::vector<std::string> & servers, const char * listen = nullptr);
	private:
        std::unordered_map<std::string, std::vector<int>> mRpcServers;
		std::unordered_map<int, std::unique_ptr<LocationUnit>> mServers;
		std::unordered_map<long long, std::unique_ptr<LocationUnit>> mClients;
	};
}

#endif //APP_LOCATIONCOMPONENT_H
