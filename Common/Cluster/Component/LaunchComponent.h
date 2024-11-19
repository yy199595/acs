//
// Created by zmhy0073 on 2022/10/12.
//

#ifndef APP_LAUNCHCOMPONENT_H
#define APP_LAUNCHCOMPONENT_H
#include "Entity/Component/Component.h"
#include "Server/Component/ITcpComponent.h"
namespace acs
{
	class LaunchComponent final : public Component, public IDestroy
    {
    public:
        LaunchComponent() = default;
        ~LaunchComponent() final = default;
	private:
		bool AddComponent(const std::vector<std::string> & service);
		bool AddRpcService(const std::vector<std::string> & service);
		bool AddHttpService(const std::vector<std::string> & service);
    private:
        bool Awake() final;
		void OnDestroy() final;
		bool LateAwake() final;
		bool LoadListenConfig();
	private:
		std::vector<ListenConfig> mTcpListens;
    };
}


#endif //APP_LAUNCHCOMPONENT_H
