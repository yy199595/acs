#pragma once
#include<set>
#include<vector>
#include<unordered_map>
#include<rapidjson/document.h>
#include<Define/CommonTypeDef.h>
namespace SoEasy
{
	class ServerConfig
	{
	public:
		ServerConfig(const std::string path);
	public:
		bool InitConfig();
	public:
		bool HasValue(const std::string k2);
		bool GetValue(const std::string k2, int & data);
		bool GetValue(const std::string k2, bool & data);
		bool GetValue(const std::string k2, float & data);
		bool GetValue(const std::string k2, double & data);
		bool GetValue(const std::string k2, long long & data);
		bool GetValue(const std::string k2, std::string & data);
		bool GetValue(const std::string k2, unsigned int & data);
		bool GetValue(const std::string k2, unsigned short & data);
		bool GetValue(const std::string k2, unsigned long long & data);
		bool GetValue(const std::string k2, std::set<std::string> & data);
		bool GetValue(const std::string k2, std::vector<std::string> & data);
	private:
		rapidjson::Value * GetJsonValue(const std::string & k2);
	private:
		const std::string mConfigPath;
		shared_ptr<rapidjson::Document> mConfigDocument;
		std::unordered_map<std::string, rapidjson::Value *> mMapConfigData;
	};
}