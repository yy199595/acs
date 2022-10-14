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
    class LocationComponent : public Component
    {
    public:
        LocationComponent() = default;
        ~LocationComponent() = default;
    public:
        bool DelLocation(const std::string & address);
        LocationUnit * GetLocationUnit(long long id) const;
        bool DelLocation(const std::string & service, const std::string & address);
        void AddLocation(const std::string & service, const std::string & address);
    public:
        size_t GetHostSize(const std::string & service) const;
        bool HasLocation(const std::string & service, const std::string & address);
        bool GetLocationss(const std::string & service, std::vector<std::string> & hosts);
    public:
        virtual bool AllotLocation(const std::string & service, std::string & address);
    private:
        std::unordered_map<std::string, std::vector<HostCounter>> mServiceLocations;
        std::unordered_map<long long, std::unique_ptr<LocationUnit>> mUnitLocations;
    };
}

#endif //APP_LOCATIONCOMPONENT_H
