//
// Created by zmhy0073 on 2022/8/12.
//

#ifndef APP_LOCATIONCOMPONENT_H
#define APP_LOCATIONCOMPONENT_H
#include<vector>
#include<string>
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
        void AddLocation(const std::string & address);
        bool DelLocation(const std::string & address);
        bool AddLocation(long long userId, const std::string & address);
    public:
        bool HasLocation(const std::string & address);
        bool GetHosts(std::vector<std::string> & hosts);
        bool GetLocation(std::string & address, int max = 100);
        bool GetLocation(long long userId, std::string & address);
        size_t GetHostSize() const { return this->mHosts.size(); }
        virtual const std::string & GetServiceName() = 0;
    protected:
        virtual void OnAddHost(const std::string & address) { }
        virtual void OnDelHost(const std::string & address) { }
    private:
        std::vector<HostCounter *> mHosts;
        std::unordered_map<long long, std::string> mUnitLocations;
    };
}

#endif //APP_LOCATIONCOMPONENT_H
