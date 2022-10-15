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
    class HostCounter
    {
    public:
        HostCounter(const std::string & address)
            : Count(0), Address(address) { }
    public:
        unsigned int Count;
        const std::string Address;
    };
}

namespace Sentry
{
	class LocationComponent : public Component, public ILuaRegister
    {
    public:
        LocationComponent() = default;
        ~LocationComponent() = default;
    public:
        bool DelLocation(const std::string & address);
		bool AllotLocation(const std::string & service, std::string & address);
        bool DelLocation(const std::string & service, const std::string & address);
        void AddLocation(const std::string & service, const std::string & address);
	 public:
		bool DelLocationUnit(long long id);
		LocationUnit * AddLocationUnit(long long id);
		LocationUnit * GetLocationUnit(long long id) const;
		LocationUnit * AddLocationUnit(long long id, const std::string & address);
	 public:
		int GetAllotCount(const std::string & address) const;
        size_t GetHostSize(const std::string & service) const;
        bool HasLocation(const std::string & service, const std::string & address);
        bool GetLocationss(const std::string & service, std::vector<std::string> & hosts);
	 private:
		void OnLuaRegister(Lua::ClassProxyHelper &luaRegister) final;
	 private:
		std::unordered_map<std::string, int> mAllotCount;
        std::unordered_map<std::string, std::vector<std::string>> mServiceLocations;
        std::unordered_map<long long, std::unique_ptr<LocationUnit>> mUnitLocations;
    };
}

#endif //APP_LOCATIONCOMPONENT_H
