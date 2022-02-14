#include<utility>
#include"ServerConfig.h"
#include<Define/CommonLogDef.h>
#include<Util/FileHelper.h>
#include"Util/DirectoryHelper.h"
namespace Sentry
{
    ServerConfig::ServerConfig(std::string  path)
            : mConfigPath(std::move(path))
    {
        this->mNodeId = 0;
    }

    bool ServerConfig::LoadConfig()
    {
        std::string outString;
        if (!Helper::File::ReadTxtFile(this->mConfigPath, outString))
        {
            throw std::logic_error("not find config : " + mConfigPath);
            return false;
        }
        mConfigDocument.Parse(outString.c_str(), outString.size());
        if (this->mConfigDocument.HasParseError())
        {
            throw std::logic_error("parse json : " + mConfigPath + " failure");
            return false;
        }
        auto iter = this->mConfigDocument.MemberBegin();
        for (; iter != this->mConfigDocument.MemberEnd(); iter++)
        {
            const std::string key = iter->name.GetString();
            rapidjson::Value* value = &iter->value;
            this->mMapConfigData.emplace(key, value);
        }
        IF_THROW_ERROR(this->GetValue("area_id", this->mNodeId));
        IF_THROW_ERROR(this->GetValue("node_name", this->mNodeName));
        return true;
    }

    bool ServerConfig::HasValue(const std::string & k2) const
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        return value != nullptr && !value->IsNull();
    }

    bool ServerConfig::GetValue(const std::string & k2, int &data) const
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsInt())
        {
            data = value->GetInt();
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string & k2, bool &data) const
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsBool())
        {
            data = value->GetBool();
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string & k2, std::string &data) const
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

    bool ServerConfig::GetValue(const std::string & k2, unsigned short &data) const
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsUint())
        {
            data = (unsigned short) value->GetUint();
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string & k2, std::set<std::string> &data) const
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

    bool ServerConfig::GetValue(const std::string & k2, std::vector<std::string> &data) const
    {
        rapidjson::Value *value = this->GetJsonValue(k2);
        if (value && value->IsArray())
        {
            for (auto iter = value->Begin(); iter != value->End(); iter++)
            {
                if (!iter->IsString())
                {
                    return false;
                }
                data.emplace_back(iter->GetString(), iter->GetStringLength());
            }
            return true;
        }
        return false;
    }

    bool ServerConfig::GetValue(const std::string & k2, std::unordered_map<std::string, std::string> &data) const
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

    bool ServerConfig::GetValue(const std::string & k1, const std::string & k2, int &data) const
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

    bool ServerConfig::GetValue(const std::string & k1, const std::string & k2, std::vector<std::string> & value) const
    {
        rapidjson::Value *json = this->GetJsonValue(k1);
        if (json == nullptr || !json->IsObject())
        {
            return false;
        }
        auto iter = json->FindMember(k2.c_str());
        if(iter == json->MemberEnd() || !iter->value.IsArray())
        {
            return false;
        }

        for(auto iter1 = iter->value.Begin(); iter1 != iter->value.End(); iter1++)
        {
            if(!iter1->IsString())
            {
                return false;
            }
            value.emplace_back(iter1->GetString());
        }

        return true;
    }

    bool ServerConfig::GetValue(const std::string & k1, const std::string & k2, std::string &value) const
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

    bool ServerConfig::GetValue(const std::string & k1, const std::string & k2, unsigned short & value) const
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

    rapidjson::Value *ServerConfig::GetJsonValue(const std::string & k2) const
    {
        auto iter = this->mMapConfigData.find(k2);
        return iter != this->mMapConfigData.end() ? iter->second : nullptr;
    }

	rapidjson::Value *ServerConfig::GetJsonValue(const std::string & k1, const std::string & k2) const
	{
		auto iter = this->mMapConfigData.find(k2);
		if (iter != this->mMapConfigData.end())
		{
			if (!iter->second->IsObject())
			{
				return nullptr;
			}
			auto iter1 = iter->second->FindMember(k2.c_str());
			return iter1 != iter->second->MemberEnd() ? &iter1->value : nullptr;
		}
		return nullptr;
	}
}
