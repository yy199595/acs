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
        bool Del(const std::string & server);
        bool Get(std::vector<std::string> & servers);
        bool Get(const std::string & server, std::string & address) const;
        void Add(const std::string & server, const std::string & address);
        inline size_t GetLocationSize() const { return this->mLocations.size(); }
    private:
		std::string mAddress;
        std::unordered_map<std::string, std::string> mLocations;
    };
}


#endif //APP_LOCATIONUNIT_H
