//
// Created by leyi on 2023/6/16.
//

#ifndef APP_JSONCONFIG_H
#define APP_JSONCONFIG_H
#include"rapidjson/document.h"
#include"Config/Base/TextConfig.h"

namespace Tendo
{
	class JsonConfig : public TextConfig
	{
	public:
		using TextConfig::TextConfig;
	private:
		bool OnLoadText(const char *str, size_t length) final;
		bool OnReloadText(const char *str, size_t length) final;
	protected:
		virtual bool OnLoadJson(rapidjson::Document & document) = 0;
		virtual bool OnReLoadJson(rapidjson::Document & document) = 0;
	};
}


#endif //APP_JSONCONFIG_H
