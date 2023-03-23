//
// Created by yjz on 2023/3/23.
//

#ifndef APP_COMMON_SERVER_CONFIG_CSVTEXTCONFIG_H
#define APP_COMMON_SERVER_CONFIG_CSVTEXTCONFIG_H
#include"TextConfig.h"
#include<vector>
namespace Sentry
{
	class CsvTextConfig : public TextConfig
	{
	public:
		explicit CsvTextConfig(const std::string & name);
	private:
		bool OnLoadText(const char *str, size_t length) final;
		bool OnReloadText(const char *str, size_t length) final;
	protected:
		virtual bool OnLoadLine(const std::unordered_map<std::string, std::string> &) = 0;
		virtual bool OnReLoadLine(const std::unordered_map<std::string, std::string> &) = 0;
	};
}

#endif //APP_COMMON_SERVER_CONFIG_CSVTEXTCONFIG_H
