//
// Created by yjz on 2022/10/27.
//
#include"httpHead.h"
namespace Http
{
    bool Head::Add(const std::string &k, int v)
    {
        auto iter = this->mHeads.find(k);
        if(iter != this->mHeads.end())
        {
            return false;
        }
        this->mHeads.emplace(k, std::to_string(v));
        return true;
    }

    bool Head::Add(const std::string &k, const std::string &v)
    {
        auto iter = this->mHeads.find(k);
        if(iter != this->mHeads.end())
        {
            return false;
        }
        this->mHeads.emplace(k, v);
        return true;
    }

    bool Head::Get(const std::string &k, std::string &v) const
    {
        auto iter = this->mHeads.find(k);
        if(iter == this->mHeads.end())
        {
            return false;
        }
        v = iter->second;
        return true;
    }

    bool Head::OnRead(std::istream &buffer)
    {
        std::string lineData;
        while(std::getline(buffer, lineData))
        {
            if(lineData == "\r")
            {
                return true;
            }
            size_t pos = lineData.find(":");
            if (pos != std::string::npos)
            {
                size_t length = lineData.size() - pos - 2;
                std::string key = lineData.substr(0, pos);
                this->mHeads.emplace(key, lineData.substr(pos + 1, length));
            }
        }
        return false;
    }

    int Head::OnWrite(std::ostream &buffer)
    {
        for(auto iter = this->mHeads.begin(); iter != this->mHeads.end(); iter++)
        {
            const std::string & key = iter->first;
            const std::string & value = iter->second;
            buffer << key << ':' << value << "\r\n";
        }
        return 0;
    }
}