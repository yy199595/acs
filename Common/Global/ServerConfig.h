#pragma once

#include<set>
#include<vector>
#include<unordered_map>
#include"Json/JsonReader.h"
#include<Define/CommonTypeDef.h>

namespace Sentry
{
    class ServerConfig : public Json::Reader
    {
    public:
        explicit ServerConfig(std::string  path);
    public:
        bool LoadConfig();
        int GetNodeId() { return this->mNodeId; }
        const std::string& GetNodeName() { return this->mNodeName; }
    private:   
        int mNodeId;
        std::string mNodeName;
        const std::string mConfigPath;
    };
}