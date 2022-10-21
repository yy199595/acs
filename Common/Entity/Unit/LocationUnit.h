//
// Created by zmhy0073 on 2022/10/14.
//

#ifndef APP_LOCATIONUNIT_H
#define APP_LOCATIONUNIT_H
#include"Unit.h"
#include<string>
#include<unordered_map>
namespace Sentry
{
	class LocationUnit : public Unit
    {
    public:
        LocationUnit(long long id);
        LocationUnit(long long id, const std::string & address);
    public:
        const std::string & GetLocation() const { return this->mAddress; }
    public:
        bool Del(const std::string & service);
        bool Get(const std::string & service, std::string & address) const;
        void Add(const std::string & service, const std::string & address);
    private:
        std::string mAddress;
        std::unordered_map<std::string, std::string> mLocations;
    };
}


#endif //APP_LOCATIONUNIT_H
