#pragma once
#include <memory>
#include <Manager/Manager.h>
#include <Protocol/com.pb.h>
using namespace std;
using namespace PB;
namespace SoEasy
{
	class NetWorkWaitCorAction;
	class ServiceBase : public Object
	{
	public:
		ServiceBase();
		virtual ~ServiceBase() {}

	public:
		virtual bool OnInit() { return false; }
		virtual void OnInitComplete() {};
		virtual bool IsLuaService() { return false; };
	public:
		bool IsInit() { return this->mIsInit; }
		virtual bool HasMethod(const std::string &method) = 0;
		virtual void OnRefreshService() {}; //刷新服务表调用
		const std::string &GetServiceName() { return this->mServiceName; };
	public:
		virtual XCode InvokeMethod(const SharedPacket, SharedPacket) = 0;
		virtual XCode InvokeMethod(const std::string &address, const SharedPacket, SharedPacket) = 0;
	public:
		void InitService(const std::string &name);
	private:
		bool mIsInit;
		std::string mServiceName;
	};
}