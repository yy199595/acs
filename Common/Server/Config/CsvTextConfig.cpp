//
// Created by yjz on 2023/3/23.
//
#include<sstream>
#include"CsvTextConfig.h"
#include"String/StringHelper.h"
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
		std::unordered_map<std::string, std::string> contents;
		if(Helper::Str::Split(line, ",", fields) == 0)
		{
			return false;
		}
		size_t size = fields.size();
		while(std::getline(fileStream, line))
		{
			values.clear();
			contents.clear();
			if(Helper::Str::Split(line, ",", values) != size)
			{
				return false;
			}
			for(size_t index = 0;index<values.size();index++)
			{
				const std::string & key = fields.at(index);
				const std::string & value = values.at(index);
				contents.emplace(key, value);
			}
			if(!this->OnReLoadLine(contents))
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