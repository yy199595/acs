//
// Created by leyi on 2023/6/16.
//

#include"JsonConfig.h"
#include"Util/File/FileHelper.h"
#include"Log/Common/CommonLogDef.h"
namespace Tendo
{
	bool JsonConfig::OnLoadText(const char* str, size_t length)
	{
		rapidjson::Document document;
		if(document.Parse(str, length).HasParseError())
		{
			LOG_ERROR("load json fail : " << this->Path());
			return false;
		}
		return this->OnLoadJson(document);
	}

	bool JsonConfig::OnReloadText(const char* str, size_t length)
	{
		rapidjson::Document document;
		if(document.Parse(str, length).HasParseError())
		{
			LOG_ERROR("reload json fail : " << this->Path());
			return false;
		}
		return this->OnReLoadJson(document);
	}
}