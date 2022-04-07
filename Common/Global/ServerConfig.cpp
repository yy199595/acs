#include<utility>
#include"ServerConfig.h"
#include<Define/CommonLogDef.h>
#include<Util/FileHelper.h>
#include<spdlog/fmt/fmt.h>
#include"Util/DirectoryHelper.h"
namespace Sentry
{
    ServerConfig::ServerConfig(std::string  path)
            : mConfigPath(std::move(path))
    {
        this->mNodeId = 0;
    }

    bool ServerConfig::LoadConfig()
    {
        std::string outString;
        if (!Helper::File::ReadTxtFile(this->mConfigPath, outString))
        {
            throw std::logic_error("not find config : " + mConfigPath);
            return false;
        }
		if(!this->ParseJson(outString))
		{
			throw std::logic_error("parse json : " + mConfigPath + " failure");
			return false;
		}
		IF_THROW_ERROR(this->GetJsonValue("listener", "rpc"));
		IF_THROW_ERROR(this->GetJsonValue("listener", "http"));
        IF_THROW_ERROR(this->GetMember("area_id", this->mNodeId));
        IF_THROW_ERROR(this->GetMember("node_name", this->mNodeName));
		return true;
    }

	std::string ServerConfig::GetRpcAddress() const
	{
		const rapidjson::Value * jsonValue = this->GetJsonValue("listener", "rpc");
		const char * ip = jsonValue->FindMember("ip")->value.GetString();
		unsigned short port = (unsigned short)jsonValue->FindMember("port")->value.GetInt();
		return fmt::format("{0}:{1}", ip, port);
	}

	std::string ServerConfig::GetHttpAddress() const
	{
		const rapidjson::Value * jsonValue = this->GetJsonValue("listener", "http");
		const char * ip = jsonValue->FindMember("ip")->value.GetString();
		unsigned short port = (unsigned short)jsonValue->FindMember("port")->value.GetInt();
		return fmt::format("{0}:{1}", ip, port);
	}
}
