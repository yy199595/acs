//
// Created by yjz on 2023/3/23.
//
#include<sstream>
#include<fstream>
#include <utility>
#include"Util/Md5/MD5.h"
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
	CsvTextConfig::CsvTextConfig(std::string  name)
		: mName(std::move(name))
	{

	}

	bool CsvTextConfig::ReloadConfig()
	{
		std::ifstream fileStream(this->mPath, std::ios::out);
		if(!fileStream.is_open())
		{
			return false;
		}
		std::string md5 = Helper::Md5::GetMd5(fileStream);
		if(this->mMd5 == md5)
		{
			return true;
		}

		std::string line;
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
			if(line[0] == '#') //忽略注释行
			{
				continue;
			}
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
		this->mMd5 = md5;
		return true;
	}

	bool CsvTextConfig::LoadConfig(const std::string& path)
	{
		std::ifstream fileStream(path, std::ios::out);
		if(!fileStream.is_open())
		{
			return false;
		}
		std::string line;
		if(!std::getline(fileStream, line))
		{
			return false;
		}
		if(line.back() == '\r')
		{
			line.pop_back();
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
			if(line[0] == '#') //忽略注释行
			{
				continue;
			}
			values.clear();
			if(line.back() == '\r')
			{
				line.pop_back();
			}
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
			if(!this->OnLoadLine(lineData))
			{
				return false;
			}
		}
		this->mPath = path;
		this->mMd5 = Helper::Md5::GetMd5(fileStream);
		return true;
	}
}