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
	public:
		inline int GetFps() { return this->mFps; }
		inline int GetServerID() { return this->mServerID; }
		inline int GetServerAreaID() { return this->mServerAreaId; }
		inline unsigned short GetServerPort() { return this->mServerPort; }
		inline unsigned int GetRecvBufferCount() { return this->mRecvBufferCount; }
		inline unsigned int GetSendBufferCount() { return this->mSendBufferCount; }
		inline unsigned short GetCenterServerPort() { return this->mCenterSrvPort; }
	public:
		inline const std::string & GetServerIP() { return this->mServerIP; }
		inline const std::string & GetServerName() { return this->mServerName; }
		inline const std::string & GetCenterServerIP() { return this->mCenterSrvIP; }
	public:
		bool FindIpFromWhiteList(const std::string & ip);
	private:
		rapidjson::Value * GetJsonValue(const std::string & k2);
	private:
		int mFps;
		int mServerID;
		int mServerAreaId;	
		std::string mServerIP;
		std::string mServerName;
		std::string mCenterSrvIP;
		std::string mConfigPath;
		unsigned short mServerPort;
		unsigned short mCenterSrvPort;
		unsigned int mRecvBufferCount;
		unsigned int mSendBufferCount;
		std::set<std::string> mWhiteListVec;
	private:
		rapidjson::Document mConfigDocument;
		std::unordered_map<std::string, rapidjson::Value *> mMapConfigData;
	};
}