#pragma once

#include<set>
#include<vector>
#include<unordered_map>
#include"Util/JsonHelper.h"
#include<Define/CommonTypeDef.h>

namespace Sentry
{
    class ServerConfig : public RapidJsonReader
    {
    public:
        explicit ServerConfig(std::string  path);
    public:
        bool LoadConfig();
        int GetNodeId() { return this->mNodeId; }
        const std::string& GetNodeName() { return this->mNodeName; }
    public:
        bool HasValue(const std::string & k2) const;

        bool GetValue(const std::string & k2, int &data) const;

        bool GetValue(const std::string & k2, bool &data) const;

        bool GetValue(const std::string & k2, std::string &data) const;

        bool GetValue(const std::string & k2, unsigned short &data) const;

        bool GetValue(const std::string & k2, std::set<std::string> &data) const;

        bool GetValue(const std::string & k2, std::vector<std::string> &data) const;

        bool GetValue(const std::string & k2, std::unordered_map<std::string, std::string> &data) const;

    public:
        bool GetValue(const std::string & k1, const std::string & k2, int &value) const;

        bool GetValue(const std::string & k1, const std::string & k2, std::string &value) const;

        bool GetValue(const std::string & k1, const std::string & k2, unsigned short & value) const;
        bool GetValue(const std::string & k1, const std::string & k2, std::vector<std::string> & value) const;


		rapidjson::Value *GetJsonValue(const std::string & k1) const;
		rapidjson::Value *GetJsonValue(const std::string & k1, const std::string & k2) const;

    private:   
        int mNodeId;
        std::string mNodeName;
        const std::string mConfigPath;
        rapidjson::Document mConfigDocument;
        std::unordered_map<std::string, rapidjson::Document> mConfigMap;
        std::unordered_map<std::string, rapidjson::Value *> mMapConfigData;
    };
}