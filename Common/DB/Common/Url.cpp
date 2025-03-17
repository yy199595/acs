//
// Created by 64658 on 2025/3/15.
//

#include <regex>
#include <utility>
#include "Url.h"
#include "Util/Tools/String.h"
#include "bundled/format.h"

namespace db
{
	Url::Url(std::string  proto)
		: mProto(std::move(proto))
	{

	}

	bool Url::Decode(const std::string& address)
	{
		std::string url = address;
		size_t pos = url.find('?');
		if(pos != std::string::npos)
		{
			url = address.substr(0, pos);
			std::vector<std::string> result;
			std::string encoded = address.substr(pos + 1);
			help::Str::Split(encoded, '&', result);
			for (const std::string& filed: result)
			{
				size_t pos1 = filed.find('=');
				if (pos1 == std::string::npos)
				{
					return false;
				}
				std::string key = filed.substr(0, pos1);
				std::string val = filed.substr(pos1 + 1);
				this->mItems.emplace(key, val);
			}
		}
		std::cmatch what;
		std::regex pattern(fmt::format("({})://([^/ :]+):?([^/ ]*)/(.*)?", this->mProto));
		if (!std::regex_match(url.c_str(), what, pattern))
		{
			return false;
		}
		std::string ip = std::string(what[2].first, what[2].second);
		std::string db = std::string(what[4].first, what[4].second);
		std::string port = std::string(what[3].first, what[3].second);
		{
			this->Add("db", db);
			this->Add("address", fmt::format("{}:{}", ip, port));
		}
		return true;
	}

	bool Url::Add(const std::string& key, const std::string& value)
	{
		if(key.empty() || value.empty())
		{
			return false;
		}
		this->mItems.emplace(key, value);
		return true;
	}

	bool Url::Get(const std::string& key, std::string& value)
	{
		auto iter = this->mItems.find(key);
		if(iter == this->mItems.end())
		{
			return false;
		}
		value = iter->second;
		return true;
	}
}