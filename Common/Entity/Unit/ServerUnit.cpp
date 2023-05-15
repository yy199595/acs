//
// Created by zmhy0073 on 2022/10/14.
//

#include"ServerUnit.h"
namespace Tendo
{

    bool ServerUnit::Del(const std::string &service)
    {
        auto iter = this->mLocations.find(service);
        if(iter == this->mLocations.end())
        {
            return false;
        }
        this->mLocations.erase(iter);
        return true;
    }
    void ServerUnit::Add(const std::string &server, const std::string &address)
    {
        this->mLocations[server] = address;
    }

    bool ServerUnit::Get(std::vector<std::string> &servers)
    {
        auto iter = this->mLocations.begin();
        for (; iter != this->mLocations.end(); iter++)
        {
            servers.emplace_back(iter->first);
        }
        return !servers.empty();
    }

    bool ServerUnit::Get(const std::string &server, std::string &address) const
    {
        auto iter = this->mLocations.find(server);
        if(iter == this->mLocations.end())
        {
            return false;
        }
        address = iter->second;       
        return true;
    }

    bool ServerUnit::Get(std::unordered_map<std::string, std::string> &servers) const
    {
        if(this->mLocations.empty())
        {
            return false;
        }
        auto iter = this->mLocations.begin();
        for(; iter != this->mLocations.end(); iter++)
        {
            servers.emplace(iter->first, iter->second);
        }
        return true;
    }
}
