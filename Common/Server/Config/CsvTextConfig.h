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
	class CsvTextConfig : public ITextConfig
	{
	public:
		explicit CsvTextConfig(std::string  name);
	public:
		bool ReloadConfig() final;
		bool LoadConfig(const std::string &path) final;
		const std::string & GetName() const final { return this->mName; }
	protected:
		virtual bool OnLoadLine(const CsvLineData & lineData) = 0;
		virtual bool OnReLoadLine(const CsvLineData & lineData) = 0;
	private:
		std::string mMd5;
		std::string mName;
		std::string mPath;
	};
}

#endif //APP_COMMON_SERVER_CONFIG_CSVTEXTCONFIG_H
