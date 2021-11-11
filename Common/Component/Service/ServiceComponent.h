#pragma once

#include <memory>
#include <Component/Component.h>

using namespace std;
using namespace com;


namespace GameKeeper
{
	class ServiceMethod;
	class CppCallHandler;

	class ServiceComponent : public Component
	{
	public:
		ServiceComponent() = default;

		virtual ~ServiceComponent() override = default;

	public:
		int GetPriority() override { return 1000; }
		virtual bool IsLuaService() { return false; };
	
	public:
		virtual void OnRefreshService() {}; //刷新服务表调用
		virtual const std::string &GetServiceName() = 0;
	public:
		bool AddMethod(ServiceMethod * method);
		bool HasMethod(const std::string &method);
		ServiceMethod * GetMethod(const std::string &method);
        long long GetCurSocketId() const { return this->mCurSocketId;}
        void SetCurSocketId(long long socketId) { this->mCurSocketId = socketId;}
	private:
        long long mCurSocketId;
		std::unordered_map<std::string, ServiceMethod *> mMethodMap;
		std::unordered_map<std::string, ServiceMethod *> mLuaMethodMap;
	};
}