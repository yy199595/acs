//
// Created by yy on 2024/7/5.
//

#ifndef APP_WATCHCOMPONENT_H
#define APP_WATCHCOMPONENT_H
#include "Core/Process/Process.h"
#include "Entity/Component/Component.h"
namespace acs
{
	class WatchComponent : public Component, public ISecondUpdate
	{
	public:
		WatchComponent();
		~WatchComponent() = default;
	private:
		void OnSecondUpdate(int tick) final;
	public:
		bool CloseProcess(long long pid);
		bool RestartProcess(long long pid);
		void GetAllProcess(json::w::Document& response);
		bool StartProcess(const std::string & name, const std::string & exe, const std::string & args);
		bool StartProcess(const std::string& name, const std::string & exe, const std::vector<std::string> & args);
	private:
		std::vector<os::Process> mProcess;
	};
}


#endif //APP_WATCHCOMPONENT_H
