
#ifdef __OS_WIN__
#include<direct.h>
#else
#endif //
#include<regex>
#include"ServerConfig.h"
#include"Core/System/System.h"
#include"Log/Common/CommonLogDef.h"
#include"Util/File/FileHelper.h"
#include"Util/String/StringHelper.h"
#include"Util/File/DirectoryHelper.h"

namespace Tendo
{
	ServerConfig::ServerConfig()
		: TextConfig("ServerConfig")
	{
		this->mServerId = 0;
		this->mGroupId = 0;
		this->mUseLua = false;
	}

	bool ServerConfig::OnReloadText(const char* str, size_t length)
	{
		return true;
	}

	bool ServerConfig::OnLoadText(const char* str, size_t length)
	{
		if (!this->ParseJson(str, length))
		{
			CONSOLE_LOG_ERROR("parse " << this->Path() << " failure");
			return false;
		}
		assert(this->GetMember("name", this->mName));
		assert(this->GetMember("id", this->mServerId));
        this->mEnvVals["${WORK_PATH}"] = System::WorkPath();
		if(this->HasMember("lua"))
		{
			this->GetMember("lua", "main", this->mMainLua);
			this->GetMember("lua", "path", this->mLuaRequires);
			this->mUseLua = true;
		}
		if (this->HasMember("listen"))
		{
			const rapidjson::Value * document = this->GetJsonValue("listen");
			for (auto iter1 = document->MemberBegin(); iter1 != document->MemberEnd(); iter1++)
			{
				ListenConfig listenConfig;
				const std::string key(iter1->name.GetString());
				const std::string address(iter1->value.GetString());
				if(!Helper::Str::SplitAddr(address, listenConfig.Net, listenConfig.Ip, listenConfig.Port))
				{
					return false;
				}
				this->mLocations.emplace(key, address);
				this->mListens.emplace(key, listenConfig);
			}
		}

		if (this->HasMember("path") && (*this)["path"].IsObject())
		{
			const rapidjson::Value& jsonObject = (*this)["path"];
			auto iter1 = jsonObject.MemberBegin();
			for (; iter1 != jsonObject.MemberEnd(); iter1++)
			{
				const std::string key(iter1->name.GetString());
				this->mPaths.emplace(key, std::string(iter1->value.GetString()));
			}
		}
        for(auto & mPath : this->mPaths)
        {
            const std::string & value = mPath.second;
            for(auto & mEnvVal : this->mEnvVals)
            {
                const std::string & k = mEnvVal.first;
                const std::string & v = mEnvVal.second;
                Helper::Str::ReplaceString(mPath.second, k, v);
            }
        }

		for(std::string & value : this->mLuaRequires)
		{
			for(auto & mEnvVal : this->mEnvVals)
			{
				const std::string & k = mEnvVal.first;
				const std::string & v = mEnvVal.second;
				Helper::Str::ReplaceString(value, k, v);
			}
		}
		for(auto & mEnvVal : this->mEnvVals)
		{
			const std::string & k = mEnvVal.first;
			const std::string & v = mEnvVal.second;
			Helper::Str::ReplaceString(this->mMainLua, k, v);
		}
		return true;
	}

	bool ServerConfig::GetListen(const std::string& name, unsigned short& listen) const
	{
		auto iter = this->mListens.find(name);
		if (iter == this->mListens.end())
		{
			return false;
		}
		listen = iter->second.Port;
		return true;
	}

	bool ServerConfig::GetListen(const std::string& name, std::string& net, unsigned short& port) const
	{
		auto iter = this->mListens.find(name);
		if (iter == this->mListens.end())
		{
			return false;
		}
		net = iter->second.Net;
		port = iter->second.Port;
		return true;
	}

	bool ServerConfig::GetListen(std::vector<std::string>& names) const
	{
		auto iter = this->mLocations.begin();
		for(; iter != this->mLocations.end(); iter++)
		{
			names.emplace_back(iter->first);
		}
		return true;
	}

	bool ServerConfig::GetLocation(const char* name, std::string& location) const
	{
		auto iter = this->mLocations.find(name);
		if (iter == this->mLocations.end())
		{
			return false;
		}
		location = iter->second;
		return true;
	}

	bool ServerConfig::GetPath(const std::string& name, std::string& path) const
	{
		auto iter = this->mPaths.find(name);
		if (iter != this->mPaths.end())
		{
			path = iter->second;
			return true;
		}
		return false;
	}
}
