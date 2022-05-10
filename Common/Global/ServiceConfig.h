#pragma once

#include<vector>
#include<shared_mutex>
#include<unordered_map>

#include"XCode/XCode.h"
#include<rapidjson/document.h>
#include"Other/InterfaceConfig.h"
namespace Sentry
{
	struct CodeConfig
	{
	 public:
		int Code;
		std::string Name;
		std::string Desc;
	};
	class ServiceConfig
	{
	 public:
		ServiceConfig() = default;
		~ServiceConfig() = default;
	 public:
		bool LoadConfig(const std::string & path);
		void GetService(std::vector<std::string> & services);
		bool HasServiceMethod(const std::string& service, const std::string& method) const;
	 public:
		const CodeConfig* GetCodeConfig(int code) const;
		const RpcInterfaceConfig* GetInterfaceConfig(int methodId) const;
		const RpcInterfaceConfig* GetInterfaceConfig(const std::string& fullName) const;
		const HttpInterfaceConfig * GetHttpIterfaceConfig(const std::string & path) const;
		void GetMethods(const std::string & service, std::vector<std::string> & methods) const;

#ifdef __DEBUG__
		void DebugCode(XCode code);
		std::string GetCodeDesc(XCode code) const;
#endif
	 private:
		bool LoadCodeConfig();
		bool LoadRpcInterface(const std::string & service, const std::string & method, const rapidjson::Value & jsonValue);
		bool LoadHttpInterface(const std::string & service, const std::string & method, const rapidjson::Value & jsonValue);
	private:
		std::string mPath;
		std::vector<std::string> mServices;
		std::unordered_map<int, CodeConfig> mCodeDescMap;
		std::unordered_map<int, RpcInterfaceConfig *> mRpcIdMap;
		std::unordered_map<std::string, RpcInterfaceConfig *> mRpcNameMap;
		std::unordered_map<std::string, HttpInterfaceConfig *> mHttpPathMap;
		std::unordered_map<std::string, std::vector<std::string>> mMethods;
	};

#define GKDebugCode(code) { App::Get().GetComponent<RpcConfigComponent>()->DebugCode(code); }
}// namespace Sentry