//
// Created by leyi on 2023/6/16.
//

#ifndef APP_JSONCONFIG_H
#define APP_JSONCONFIG_H
#include"Config/Base/TextConfig.h"
#include"Yyjson/Document/Document.h"

namespace joke
{
	class JsonConfig : public TextConfig, public json::r::Document
	{
	public:
		using TextConfig::TextConfig;
		bool OnLoadText(const char *str, size_t length) override;
		bool OnReloadText(const char *str, size_t length) override;
	private:
		bool ReloadConfig() final;
	protected:
		virtual bool OnLoadJson() = 0;
		virtual bool OnReLoadJson() = 0;
	private:
		std::string mName;
	};
}


#endif //APP_JSONCONFIG_H
