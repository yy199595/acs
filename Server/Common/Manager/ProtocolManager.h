#pragma once

#include"Manager.h"

namespace Sentry
{
	class ProtocolConfig
	{
	public:
		ProtocolConfig(const unsigned short id, const std::string & service, const std::string & method,
			const std::string & request, const std::string & response, bool client = false)
			:IsClientMessage(client), MethodName(method), MethodId(id),
			ServiceName(service), RequestMsgName(request), ResponseMsgName(response) {}
	public:
		const bool IsClientMessage;
		const std::string MethodName;
		const unsigned short MethodId;
		const std::string ServiceName;
		const std::string RequestMsgName;
		const std::string ResponseMsgName;
	};
}

namespace Sentry
{
	class ProtocolManager : public Manager
	{
	public:
		ProtocolManager() { }
		~ProtocolManager() { }
	protected:
		bool OnInit() final;
	public:
		const ProtocolConfig * GetProtocolConfig(unsigned short id) const;
		const ProtocolConfig * GetProtocolConfig(const std::string & service, const std::string & method) const;
	public:
		std::unordered_map<unsigned short, ProtocolConfig *> mProtocolMap;
		std::unordered_map<std::string, ProtocolConfig *> mProtocolNameMap;
	};
}