//
// Created by yjz on 2023/3/23.
//

#ifndef APP_COMMON_SERVER_CONFIG_CSVTEXTCONFIG_H
#define APP_COMMON_SERVER_CONFIG_CSVTEXTCONFIG_H
#include"TextConfig.h"
#include<vector>

namespace Sentry
{
	class CsvLineData
	{
	public:
		bool Get(const std::string & key, int & value) const;
		bool Get(const std::string & key, bool & value) const;
		bool Get(const std::string & key, long long & value) const;
		bool Get(const std::string & key, std::string & value) const;
	public:
		bool Add(const std::string & key, const std::string & value);
	private:
		std::unordered_map<std::string, std::string> mDatas;
	};
}

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
		virtual bool OnLoadLine(const CsvLineData & lineData) = 0;
		virtual bool OnReLoadLine(const CsvLineData & lineData) = 0;
	};
}

#endif //APP_COMMON_SERVER_CONFIG_CSVTEXTCONFIG_H
