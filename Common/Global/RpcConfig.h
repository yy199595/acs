#pragma once

#include<vector>
#include<shared_mutex>
#include<unordered_map>

#include"XCode/XCode.h"
#include"Other/ProtoConfig.h"
namespace Sentry
{
	struct CodeConfig
	{
	 public:
		int Code;
		std::string Name;
		std::string Desc;
	};
	class RpcConfig
	{
	 public:
		RpcConfig() = default;
		~RpcConfig() = default;
	 public:
		bool LoadConfig(const std::string & path);
		bool HasServiceMethod(const std::string& service, const std::string& method) const;
		bool GetMethods(const std::string& service, std::vector<std::string>& methods) const;
	 public:
		const CodeConfig* GetCodeConfig(int code) const;
		const ProtoConfig* GetProtocolConfig(int methodId) const;
		const ProtoConfig* GetProtocolConfig(const std::string& fullName) const;

#ifdef __DEBUG__
		void DebugCode(XCode code);
		std::string GetCodeDesc(XCode code) const;
#endif
	 private:
		bool LoadCodeConfig();
	 private:
		std::string mPath;
		std::unordered_map<int, CodeConfig> mCodeDescMap;
		std::unordered_map<int, ProtoConfig> mProtocolIdMap;
		std::unordered_map<std::string, ProtoConfig> mProtocolNameMap;
		std::unordered_map<std::string, std::vector<ProtoConfig>> mServiceMap;

	};

#define GKDebugCode(code) { App::Get().GetComponent<RpcConfigComponent>()->DebugCode(code); }
}// namespace Sentry