#pragma once

#include <set>
#include <vector>
#include <unordered_map>
#include <rapidjson/document.h>
#include <Define/CommonTypeDef.h>

namespace GameKeeper
{
    class ServerConfig
    {
    public:
        ServerConfig(const std::string path);

    public:
        bool InitConfig();

    public:
        bool HasValue(const std::string k2);

        bool GetValue(const std::string k2, int &data);

        bool GetValue(const std::string k2, bool &data);

        bool GetValue(const std::string k2, std::string &data);

        bool GetValue(const std::string k2, unsigned short &data);

        bool GetValue(const std::string k2, std::set<std::string> &data);

        bool GetValue(const std::string k2, std::vector<std::string> &data);

        bool GetValue(const std::string k2, std::unordered_map<std::string, std::string> &data);

    public:
        bool GetValue(const std::string k1, const std::string k2, int &value);

        bool GetValue(const std::string k1, const std::string k2, std::string &value);

        bool GetValue(const std::string k1, const std::string k2, unsigned short & value);
        bool GetValue(const std::string k1, const std::string k2, std::vector<std::string> & value);


		rapidjson::Value *GetJsonValue(const std::string k1);
		rapidjson::Value *GetJsonValue(const std::string k1, const std::string k2);

    public:
        short GetNodeId() const { return this->mNodeId; }

        short GetAreaId() const { return this->mAreaId; }

        const std::string &GetNodeName()
        { return this->mNodeName; }

    private:
        unsigned short mAreaId;
        unsigned short mNodeId;
        std::string mNodeName;
        const std::string mConfigPath;
        shared_ptr<rapidjson::Document> mConfigDocument;
        std::unordered_map<std::string, rapidjson::Value *> mMapConfigData;
    };
}