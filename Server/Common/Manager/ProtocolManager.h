#pragma once

#include "Manager.h"

#include<Other/ProtocolConfig.h>

namespace Sentry
{
	class ProtocolManager : public Manager
	{
	public:
		ProtocolManager() {}
		~ProtocolManager() {}

	protected:
		bool OnInit() final;

	public:
		const ProtocolConfig *GetProtocolConfig(unsigned short id) const;
		const ProtocolConfig *GetProtocolConfig(const std::string &service, const std::string &method) const;
	public:
		Message * CreateMessage(const std::string & name);
		Message * CreateMessage(const std::string & name, const char * msg, const size_t size);
	public:
		std::unordered_map<unsigned short, ProtocolConfig *> mProtocolMap;
		std::unordered_map<std::string, ProtocolConfig *> mProtocolNameMap;
	};
}