#pragma once
#include<CommonModule/Component.h>
namespace SoEasy
{

	struct LogicSrvInfo
	{
	public:
		LogicSrvInfo()
		{
			this->mAreaId = 0;
			this->mServerId = 0;
			this->mServerPort = 0;
		}
	public:
		int mAreaId;
		int mServerId;
		std::string mServerIp;
		std::string mServerName;
		unsigned int mServerPort;
	};

	class LogicServerDataInfo : public Component
	{
	public:
		using Component::Component;
		virtual ~LogicServerDataInfo() { }
	protected:
		void OnFrameStart() override;
	public:
		const int  GetServerAreaud()		{ return this->mSrvInfo.mAreaId; }
		const int  GetServerId()			{ return this->mSrvInfo.mServerId; }
		const std::string & GetServerIp()	{ return this->mSrvInfo.mServerIp; }
		const std::string & GetServerName() { return this->mSrvInfo.mServerName; }
		const unsigned int GetServerPort()	{ return this->mSrvInfo.mServerPort; }
	public:
		void SetLogicSrvInfo(LogicSrvInfo & info) { this->mSrvInfo = info; }
	private:
		LogicSrvInfo mSrvInfo;
	};
}