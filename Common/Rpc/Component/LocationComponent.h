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
        bool DelLocation(long long useId);
        bool DelLocation(const std::string & address);
        void AddLocation(const std::string & address);
        void AddLocation(const std::string & address, long long userId);
    public:
        bool HasLocation(const std::string & address);
        bool GetHosts(std::vector<std::string> & hosts);
        bool GetLocation(long long userId, std::string & address);
        size_t GetHostSize() const { return this->mHosts.size(); }
        virtual const std::string & GetServiceName() = 0;
    public:
        virtual bool AllotLocation(std::string & address);
        virtual bool AllotLocation(long long userId, std::string & address);
    protected:
        virtual void OnAddHost(const std::string & address) { }
        virtual void OnDelHost(const std::string & address) { }
    private:
        std::vector<HostCounter *> mHosts;
        std::unordered_map<long long, std::string> mUnitLocations;
    };
}

#endif //APP_LOCATIONCOMPONENT_H
