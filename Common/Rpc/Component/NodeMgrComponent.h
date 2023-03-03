//
// Created by zmhy0073 on 2022/8/12.
//

#ifndef APP_LOCATIONCOMPONENT_H
#define APP_LOCATIONCOMPONENT_H
#include<vector>
#include<string>
#include"Unit/LocationUnit.h"
#include"Component/Component.h"

namespace Sentry
{
	class NodeMgrComponent final : public Component, public IComplete
    {
    public:
        NodeMgrComponent() = default;
        ~NodeMgrComponent() = default;
    public:
        bool DelServer(const std::string& address);
		void WaitServerStart(const std::string & server);//等待某个服务器启动
		bool AllotServer(const std::string& server, std::string& address);
        void AddRpcServer(const std::string & server, const std::string & address);
        void AddHttpServer(const std::string& server, const std::string& address);
	 public:
		bool GetTranLocation(std::string & address);
		bool GetTranLocation(long long userId, std::string & address);
	 public:
		bool DelUnit(long long id);
		LocationUnit * GetUnit(long long id) const;
		bool AddUnit(std::unique_ptr<LocationUnit> locationUnit);
	 public:
		bool LateAwake() final;
		void OnLocalComplete() final;
		bool GetServers(std::vector<std::string> & hosts);
		bool GetServers(const std::string & server, std::vector<std::string> & hosts);
	 private:
		std::vector<std::string> mLocations;
        std::unordered_map<std::string, std::vector<std::string>> mRpcServers;
        //std::unordered_map<std::string, std::vector<std::string>> mHttpServers;
        std::unordered_map<long long, std::unique_ptr<LocationUnit>> mUnitLocations;
    };
}

#endif //APP_LOCATIONCOMPONENT_H
