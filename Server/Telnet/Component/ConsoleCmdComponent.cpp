//
// Created by yjz on 2023/4/2.
//

#include "ConsoleCmdComponent.h"
namespace Sentry
{
	void ConsoleCmdComponent::Add(const std::string& cmd, CommandMethod method)
	{
		this->mCommands[cmd] = method;
	}
	bool ConsoleCmdComponent::Invoke(const std::string& cmd, const std::string& args)
	{
		auto iter = this->mCommands.find(cmd);
		if(iter == this->mCommands.end())
		{
			return false;
		}
		iter->second(args);
		return true;
	}
}
