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
	class LocationComponent : public Component
    {
    public:
        LocationComponent() = default;
        ~LocationComponent() = default;
    public:
        bool DelServer(const std::string& address);
        size_t GetServerCount(const std::string& name);
        bool AllotServer(const std::string& server, std::string& address);
        void AddRpcServer(const std::string & server, const std::string & address);
        void AddHttpServer(const std::string& server, const std::string& address);
	 public:
		bool DelUnit(long long id);
		LocationUnit * GetUnit(long long id) const;
		bool AddUnit(std::unique_ptr<LocationUnit> locationUnit);
        size_t GetUnitCount() const { return this->mUnitLocations.size(); }
	 public:
		int GetAllotCount(const std::string & address) const;       
        bool GetServers(const std::string & server, std::vector<std::string> & hosts);
	 private:
		std::unordered_map<std::string, int> mAllotCount;
        std::unordered_map<std::string, std::vector<std::string>> mRpcServers;
        std::unordered_map<std::string, std::vector<std::string>> mHttpServers;
        std::unordered_map<long long, std::unique_ptr<LocationUnit>> mUnitLocations;
    };
}

#endif //APP_LOCATIONCOMPONENT_H
