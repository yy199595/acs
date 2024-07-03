//
// Created by yy on 2024/6/29.
//

#ifndef APP_LANGCONFIG_H
#define APP_LANGCONFIG_H
#include <string>
#include <unordered_map>
#include "Core/Singleton/Singleton.h"
namespace joke
{
	class LangConfig : public Singleton<LangConfig>
	{
	public:
		bool LoadConfig(const std::string & path);
		bool Get(const std::string & key, std::string & value);
	public:
		static const std::string Text(const std::string & key);
	private:
		std::unordered_map<std::string, std::string> mDict;
	};
}


#endif //APP_LANGCONFIG_H
