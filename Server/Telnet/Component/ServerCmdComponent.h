//
// Created by yjz on 2023/4/2.
//

#ifndef _SERVERCMDCOMPONENT_H_
#define _SERVERCMDCOMPONENT_H_
#include"ConsoleCmdComponent.h"
namespace Sentry
{
	class ServerCmdComponent : public ConsoleCmdComponent
	{
	 public:
		void Help(std::stringstream &ss) final;
	 private:
		bool Awake();
		void Stop(const std::string & args);
		void Info(const std::string & args);
	 private:
		class HttpComponent * mHttpComponent;
	};
}

#endif //_SERVERCMDCOMPONENT_H_
