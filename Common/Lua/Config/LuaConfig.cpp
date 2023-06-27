//
// Created by leyi on 2023/6/9.
//

#include"LuaConfig.h"
#include"Core/System/System.h"
#include"rapidjson/error/en.h"
#include"Log/Common/CommonLogDef.h"
namespace Tendo
{
	bool LuaConfig::Init(const rapidjson::Value& data)
	{
		try
		{
			if (data.HasMember("require"))
			{
				const rapidjson::Value& arr = data["require"];
				for (int index = 0; index < arr.Size(); index++)
				{
					std::string val(arr[index].GetString());
					{
						System::SubValue(val);
						this->mRequires.emplace_back(val);
					}
				}
			}
			if (data.HasMember("loadfile"))
			{
				const rapidjson::Value& arr = data["loadfile"];
				for (int index = 0; index < arr.Size(); index++)
				{
					std::string path(arr[index].GetString());
					{
						System::SubValue(path);
						this->mLoadfiles.emplace_back(path);
					}
				}
			}
			if (data.HasMember("main"))
			{
				this->mMain = data["main"].GetString();
				System::SubValue(this->mMain);
			}
		}
		catch (rapidjson::ParseErrorCode& code)
		{
			LOG_ERROR(rapidjson::GetParseError_En(code));
			return false;
		}
		return true;
	}
}