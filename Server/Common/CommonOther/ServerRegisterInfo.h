#pragma once
#include<string>
namespace SoEasy
{
	struct ServerRegisterInfo
	{
	public:
		int mAreaId;
		std::string mServerIp;
		std::string mServerPort;
		std::string mServerName;
		long long mRegisterTime;
		std::string mServerAddress;
	};
}