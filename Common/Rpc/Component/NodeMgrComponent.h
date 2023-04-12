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
        bool DelServer(const std::string& address);
		void WaitServerStart(const std::string & server);//等待某个服务器启动
        void AddRpcServer(const std::string & server, const std::string & address);
        void AddHttpServer(const std::string& server, const std::string& address);
	 public:
		bool DelUnit(long long id);
        bool DelServer(const std::string& server, long long id);
        bool AddRpcServer(const std::string& server, long long id, const std::string& address);
    public:
		bool GetServers(std::vector<std::string> & hosts);
        bool GetServer(const std::string & name, std::string & address);
		bool GetServers(const std::string & name, std::vector<std::string> & hosts);
        bool GetServer(const std::string & name, long long userId, std::string & address);
		bool GetServer(long long userId, std::unordered_map<std::string, std::string> & servers);
	 private:
        
        std::unordered_map<std::string, std::vector<std::string>> mRpcServers;
        std::unordered_map<long long, std::unique_ptr<LocationUnit>> mUnitLocations;
    };
}

#endif //APP_LOCATIONCOMPONENT_H
