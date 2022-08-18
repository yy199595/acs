//
// Created by zmhy0073 on 2022/8/12.
//

#ifndef APP_SERVICEHOST_H
#define APP_SERVICEHOST_H
#include<vector>
#include<string>
#include<unordered_map>

namespace Sentry
{
    class HostCounter
    {
    public:
        HostCounter(const std::string & address)
            : Address(address), Count(0) { }
    public:
        unsigned int Count;
        const std::string Address;
    };
}

namespace Sentry
{
    class ServiceHost
    {
    public:
        bool DelHost(long long unitId);
        void AddHost(const std::string & address);
        bool DelHost(const std::string & address);
        void AddHost(const std::string & address, long long unitId);
    public:
        bool HasHost(const std::string & address);
        bool GetHosts(std::vector<std::string> & hosts);
        bool GetHost(std::string & address, int max = 100);
        bool GetHost(long long unitId, std::string & address);
        size_t GetHostSize() const { return this->mHosts.size(); }
        size_t GetUnitSize() const { return this->mUnitHosts.size(); }
    protected:
        virtual void OnUnitJoin(long long unitId) { }
        virtual void OnUnitExit(long long unitId) { }
        virtual const std::string & GetLocalHost() = 0;
        virtual void OnAddHost(const std::string & address) { }
        virtual void OnDelHost(const std::string & address) { }
    private:
        std::vector<HostCounter *> mHosts;
        std::unordered_map<long long, std::string> mUnitHosts;
    };
}

#endif //APP_SERVICEHOST_H
