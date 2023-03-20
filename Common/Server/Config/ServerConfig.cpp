#include<utility>
#ifdef __OS_WIN__
#include<direct.h>
#else
#include<unistd.h>
#endif // 
#include<regex>
#include"ServerConfig.h"
#include"Log/CommonLogDef.h"
#include"File/FileHelper.h"
#include"String/StringHelper.h"
#include"File/DirectoryHelper.h"
namespace Sentry
{
	ServerConfig::ServerConfig(const std::string& server)
		: TextConfig("ServerConfig"), mName(server)
	{
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
		if (!this->HasMember(this->mName.c_str()))
		{
			return false;
		}
		auto iter = this->FindMember(this->mName.c_str());
		if (iter == this->MemberEnd())
		{
			return false;
		}
		if(iter->value.HasMember("lua"))
		{
			const rapidjson::Value& document = iter->value["lua"];
			for(auto iter = document.MemberBegin(); iter != document.MemberEnd(); iter++)
			{
				const std::string key = iter->name.GetString();
				const std::string value = iter->value.GetString();
				this->mLuaConfigs.emplace(key, value);
			}
			this->mUseLua = true;
		}
		if (iter->value.HasMember("address"))
		{
			const rapidjson::Value& document = iter->value["address"];
			for (auto iter1 = document.MemberBegin(); iter1 != document.MemberEnd(); iter1++)
			{
				std::string ip;
				unsigned short port = 0;
				const std::string key(iter1->name.GetString());
				const std::string address(iter1->value.GetString());
				if(!Helper::Str::SplitAddr(address, ip, port))
				{
					return false;
				}
				this->mListens.emplace(key, port);
				this->mLocations.emplace(key, address);
			}
		}

		if (iter->value.HasMember("path"))
		{
			const rapidjson::Value& document = iter->value["path"];
			for (auto iter1 = document.MemberBegin(); iter1 != document.MemberEnd(); iter1++)
			{
				const std::string key(iter1->name.GetString());
				const std::string value(iter1->value.GetString());
				this->mPaths.emplace(key, this->WorkPath() + value);
			}
		}

		if (this->HasMember("path") && (*this)["path"].IsObject())
		{
			const rapidjson::Value& jsonObject = (*this)["path"];
			auto iter1 = jsonObject.MemberBegin();
			for (; iter1 != jsonObject.MemberEnd(); iter1++)
			{
				const std::string key(iter1->name.GetString());
				const std::string value(iter1->value.GetString());
				this->mPaths.emplace(key, this->WorkPath() + value);
			}
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
		listen = (unsigned short)iter->second;
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
	bool ServerConfig::ParseHttpAddress(const std::string& address, unsigned short & port) const
	{
		std::cmatch what;
		std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");
		if (!std::regex_match(address.c_str(), what, pattern))
		{
			return false;
		}
		std::string protocol = std::string(what[1].first, what[1].second);
		std::string portStr = std::string(what[3].first, what[3].second);
		if (portStr.length() == 0)
		{
			port = (protocol == "http" ? 80 : 443);
			return true;
		}
		port = std::stoi(portStr);
		return true;
	}
	bool ServerConfig::GetLuaConfig(const std::string& name, std::string& value) const
	{
		auto iter = this->mLuaConfigs.find(name);
		if(iter == this->mLuaConfigs.end())
		{
			return false;
		}
		value = iter->second;
		return true;
	}
}
