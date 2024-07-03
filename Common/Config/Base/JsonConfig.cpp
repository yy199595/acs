//
// Created by leyi on 2023/6/16.
//

#include"JsonConfig.h"
#include"Log/Common/CommonLogDef.h"

namespace joke
{
	bool JsonConfig::OnLoadText(const char* str, size_t length)
	{
		if(!this->Decode(str, length))
		{
			CONSOLE_LOG_FATAL("load {} : {}", this->Path(),  this->GetError());
			return false;
		}
		return this->OnLoadJson();
	}

	bool JsonConfig::OnReloadText(const char* str, size_t length)
	{
		if(!this->Decode(str, length))
		{
			LOG_ERROR("load {} : {}", this->Path(),  this->GetError());
			return false;
		}
		return this->OnReLoadJson();
	}

	bool JsonConfig::ReloadConfig()
	{
		if(!this->FromFile(this->mPath))
		{
			LOG_ERROR("reload {} fail", this->Path());
			return false;
		}
		return this->OnReLoadJson();
	}
}