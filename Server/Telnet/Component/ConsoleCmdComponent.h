//
// Created by yjz on 2023/4/2.
//

#ifndef _CONSOLECMDCOMPONENT_H_
#define _CONSOLECMDCOMPONENT_H_
#include<sstream>
#include<functional>
#include<unordered_map>
#include<Core/Component/Component.h>
using CommandMethod = std::function<void(const std::string &)>;

namespace Sentry
{
	class ConsoleCmdComponent : public Component
	{
	 public:
		virtual void Help(std::stringstream & ss) = 0;
	 public:
		void Add(const std::string & cmd, CommandMethod method);
		bool Invoke(const std::string & cmd, const std::string & args);
	 private:
		std::unordered_map<std::string, CommandMethod> mCommands;
	};
#define BIND_COMMAND(name, func) this->Add(name, std::bind(&func, this, args1))
}

#endif //_CONSOLECMDCOMPONENT_H_
