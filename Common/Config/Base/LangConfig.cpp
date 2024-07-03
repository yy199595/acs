//
// Created by yy on 2024/6/29.
//

#include "LangConfig.h"
#include "Util/File/FileHelper.h"
namespace joke
{
	bool LangConfig::Get(const std::string& key, std::string& value)
	{
		auto iter = this->mDict.find(key);
		if(iter == this->mDict.end())
		{
			return false;
		}
		value = iter->second;
		return true;
	}

	const std::string LangConfig::Text(const std::string& key)
	{
		std::string value("未知");
		if(LangConfig::Inst()->Get(key, value))
		{

		}
		return value;
	}

	bool LangConfig::LoadConfig(const std::string& path)
	{
		if(path.empty())
		{
			return false;
		}
		std::vector<std::string> lineDatas;
		if(!help::fs::ReadTxtFile(path, lineDatas))
		{
			return false;
		}
		for(const std::string & line : lineDatas)
		{
			size_t pos = line.find(',');
			if(pos == std::string::npos)
			{
				return false;
			}
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			this->mDict.emplace(key, value);
		}
		return true;
	}
}