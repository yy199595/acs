#include <fstream>
#include "ServerConfig.h"
#include <Define/CommonDef.h>
#include <Util/FileHelper.h>

namespace Sentry
{
    ServerConfig::ServerConfig(const std::string path)
            : mConfigPath(path)
    {
        this->mAreaId = 0;
        this->mNodeId = 0;
    }

    bool ServerConfig::InitConfig()
    {
        std::string outString = "";
        this->mConfigDocument = make_shared<rapidjson::Document>();
        if (!FileHelper::ReadTxtFile(this->mConfigPath, outString))
        {
            SayNoDebugError("not find config " << mConfigPath);
            return false;
        }
        mConfigDocument->Parse(outString.c_str(), outString.size());
        if (this->mConfigDocument->HasParseError())
        {
            SayNoDebugFatal("parse " << mConfigPath << " fail");
            return false;
        }

        auto iter = this->mConfigDocument->MemberBegin();
        for (; iter != this->mConfigDocument->MemberEnd(); iter++)
        {
            const std::string key = iter->name.GetString();
            rapidjson::Value *value = &iter->value;
            this->mMapConfigData.emplace(key, value);
        }
        SayNoAssertRetFalse_F(this->GetValue("AreaId", this->mAreaId));
        SayNoAssertRetFalse_F(this->GetValue("NodeId", this->mNodeId));
        SayNoAssertRetFalse_F(this->GetValue("NodeName", this->mNodeName));
        return true;
    }

    bool ServerConfig::HasValue(const std::string k2)
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        return value != nullptr && value->IsNull() == false;
    }

    bool ServerConfig::GetValue(const std::string k2, int &data)
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsInt())
        {
            data = value->GetInt();
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string k2, bool &data)
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsBool())
        {
            data = value->GetBool();
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string k2, std::string &data)
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsString())
        {
            data.clear();
            const char *str = value->GetString();
            const size_t size = value->GetStringLength();
            data.append(str, size);
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string k2, unsigned short &data)
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsUint())
        {
            data = (unsigned short) value->GetUint();
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string k2, std::set<std::string> &data)
    {
        data.clear();
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsArray())
        {
            for (auto iter = value->Begin(); iter != value->End(); iter++)
            {
                if (iter->IsString())
                {
                    std::string str(iter->GetString(), iter->GetStringLength());
                    data.insert(str);
                }
            }
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string k2, std::vector<std::string> &data)
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsArray())
        {
            for (auto iter = value->Begin(); iter != value->End(); iter++)
            {
                if (iter->IsString())
                {
                    std::string str(iter->GetString(), iter->GetStringLength());
                    data.push_back(str);
                }
            }
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string k2, std::unordered_map<std::string, std::string> &data)
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value == nullptr || !value->IsObject())
        {
            return false;
        }
        for (auto iter = value->MemberBegin(); iter != value->MemberEnd(); iter++)
        {
            if (!iter->name.IsString() || !iter->value.IsString())
            {
                return false;
            }
            const std::string key(iter->name.GetString(), iter->name.GetStringLength());
            const std::string value(iter->value.GetString(), iter->value.GetStringLength());
            data.insert(std::make_pair(key, value));
        }
        return true;
    }

    bool ServerConfig::GetValue(const std::string k1, const std::string k2, int &data)
    {
        rapidjson::Value *value = this->GetJsonValue(k1);
        if (value == nullptr || !value->IsObject())
        {
            return false;
        }

        auto iter = value->FindMember(k2.c_str());
        if (iter == value->MemberEnd() || !iter->value.IsInt())
        {
            return false;
        }
        data = iter->value.GetInt();
        return true;
    }

    bool ServerConfig::GetValue(const std::string k1, const std::string k2, std::string &value)
    {
        rapidjson::Value  * json = this->GetJsonValue(k1);
        if(json== nullptr || !json->IsObject())
        {
            return false;
        }
        auto iter = json->FindMember(k2.c_str());
        if(iter == json->MemberEnd() || !iter->value.IsString())
        {
            return false;
        }
        value = iter->value.GetString();
        return true;
    }

    bool ServerConfig::GetValue(const std::string k1, const std::string k2, unsigned short & value)
    {
        rapidjson::Value  * json = this->GetJsonValue(k1);
        if(json== nullptr || !json->IsObject())
        {
            return false;
        }
        auto iter = json->FindMember(k2.c_str());
        if(iter == json->MemberEnd() || !iter->value.IsInt())
        {
            return false;
        }
        value = (unsigned short)iter->value.GetInt();
        return true;
    }


    rapidjson::Value *ServerConfig::GetJsonValue(const std::string &k2)
    {
        auto iter = this->mMapConfigData.find(k2);
        return iter != this->mMapConfigData.end() ? iter->second : nullptr;
    }
}
