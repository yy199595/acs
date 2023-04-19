//
// Created by zmhy0073 on 2022/10/14.
//

#ifndef APP_LOCATIONUNIT_H
#define APP_LOCATIONUNIT_H
#include<string>
#include<vector>
#include<unordered_map>
namespace Tendo
{
	class LocationUnit
    {
    public:
        LocationUnit() = default;
		LocationUnit(const std::string & name, int id) : mId(id), mName(name) { }
    public:
        bool Del(const std::string & server);
        bool Get(std::vector<std::string> & servers);
        bool Get(const std::string & server, std::string & address) const;
        void Add(const std::string & server, const std::string & address);
        bool Get(std::unordered_map<std::string, std::string> & servers) const;
        inline size_t GetLocationSize() const { return this->mLocations.size(); }
	public:
		int GetId() const { return this->mId; }
    private:
		int mId;
		std::string mName;
        std::unordered_map<std::string, std::string> mLocations;
    };
}


#endif //APP_LOCATIONUNIT_H
