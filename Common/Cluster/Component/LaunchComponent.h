//
// Created by zmhy0073 on 2022/10/12.
//

#ifndef APP_LAUNCHCOMPONENT_H
#define APP_LAUNCHCOMPONENT_H
#include"Entity/Component/Component.h"

namespace acs
{
	class LaunchComponent final : public Component
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
    };
}


#endif //APP_LAUNCHCOMPONENT_H
