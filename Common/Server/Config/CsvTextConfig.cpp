//
// Created by yjz on 2023/3/23.
//
#include<sstream>
#include"CsvTextConfig.h"
#include"Util/String/StringHelper.h"

namespace Sentry
{
	bool CsvLineData::Add(const std::string& key, const std::string& value)
	{
		auto iter = this->mDatas.find(key);
		if(iter != this->mDatas.end())
		{
			return false;
		}
		this->mDatas.emplace(key, value);
		return true;
	}

	bool CsvLineData::Get(const std::string& key, int& value) const
	{
		auto iter = this->mDatas.find(key);
		if(iter == this->mDatas.end())
		{
			return false;
		}
		value = std::stoi(iter->second);
		return true;
	}

	bool CsvLineData::Get(const std::string& key, bool& value) const
	{
		auto iter = this->mDatas.find(key);
		if(iter == this->mDatas.end())
		{
			return false;
		}
		value = iter->second == "true";
		return true;
	}

	bool CsvLineData::Get(const std::string& key, long long& value) const
	{
		auto iter = this->mDatas.find(key);
		if(iter == this->mDatas.end())
		{
			return false;
		}
		value = std::stoll(iter->second);
		return true;
	}

	bool CsvLineData::Get(const std::string& key, std::string& value) const
	{
		auto iter = this->mDatas.find(key);
		if(iter == this->mDatas.end())
		{
			return false;
		}
		value = iter->second;
		return true;
	}
}

namespace Sentry
{
	CsvTextConfig::CsvTextConfig(const std::string& name)
		: TextConfig(name.c_str())
	{

	}

	bool CsvTextConfig::OnLoadText(const char* str, size_t length)
	{
		std::string line;
		std::stringstream fileStream;
		fileStream.write(str, length);
		if(!std::getline(fileStream, line))
		{
			return false;
		}
		std::vector<std::string> fields;
		std::vector<std::string> values;
		if(Helper::Str::Split(line, ",", fields) == 0)
		{
			return false;
		}
		size_t size = fields.size();
		while(std::getline(fileStream, line))
		{
			values.clear();
			CsvLineData lineData;
			if(Helper::Str::Split(line, ",", values) != size)
			{
				return false;
			}
			for(size_t index = 0;index<values.size();index++)
			{
				const std::string & key = fields.at(index);
				const std::string & value = values.at(index);
				lineData.Add(key, value);
			}
			if(!this->OnReLoadLine(lineData))
			{
				return false;
			}
		}
		return true;
	}

	bool CsvTextConfig::OnReloadText(const char* str, size_t length)
	{
		return true;
	}
}