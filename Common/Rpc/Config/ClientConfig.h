//
// Created by leyi on 2023/6/16.
//

#ifndef APP_CLIENTCONFIG_H
#define APP_CLIENTCONFIG_H
#include"Config/Base/JsonConfig.h"
#include"Core/Singleton/Singleton.h"
namespace Tendo
{
	class ClientConfig : public JsonConfig , public ConstSingleton<ClientConfig>
	{
	public:
		ClientConfig() : JsonConfig("ClientConfig") { }
	private:
		bool OnLoadJson(rapidjson::Document &document) final;
		bool OnReLoadJson(rapidjson::Document &document) final;
	public:
		bool GetConfig(const std::string & name, std::string & response) const;
	private:
		std::unordered_map<std::string, std::string> mConfigs;
	};
}


#endif //APP_CLIENTCONFIG_H
