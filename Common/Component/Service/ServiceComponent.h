#pragma once

#include <memory>
#include <Component/Component.h>

using namespace std;
using namespace com;


namespace GameKeeper
{
	class ServiceMethod;
	class CppCallHandler;
    class JsonServiceMethod;
	class ServiceComponent : public Component
	{
	public:
		ServiceComponent() = default;
		~ServiceComponent() override = default;

	public:
		int GetPriority() override { return 1000; }
		virtual bool IsLuaService() { return false; };
	
	public:
        virtual const std::string &GetServiceName() = 0;
	public:

		bool AddMethod(ServiceMethod * method);
        bool AddMethod(JsonServiceMethod * method);
        bool HasJsonMethod(const std::string &method);
		bool HasProtoMethod(const std::string &method);
        ServiceMethod * GetProtoMethod(const std::string &method);
		JsonServiceMethod * GetJsonMethod(const std::string &method);
        long long GetCurSocketId() const { return this->mCurSocketId;}
        void SetCurSocketId(long long socketId) { this->mCurSocketId = socketId;}
	private:
        long long mCurSocketId;
		std::unordered_map<std::string, ServiceMethod *> mMethodMap;
        std::unordered_map<std::string, JsonServiceMethod *> mJsonMethodMap;
    };
    inline std::string GetFunctionName(const std::string func)
    {
        size_t pos = func.find("::");
        return func.substr(pos + 2);
    }
}