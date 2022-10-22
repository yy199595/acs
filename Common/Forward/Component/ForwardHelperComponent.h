//
// Created by zmhy0073 on 2022/10/18.
//

#ifndef APP_FORWARDHELPERCOMPONENT_H
#define APP_FORWARDHELPERCOMPONENT_H
#include"Client/Message.h"
#include"Component/Component.h"
namespace Sentry
{
	class ForwardHelperComponent final : public Component, public IComplete
    {
    public:
        ForwardHelperComponent() = default;
        ~ForwardHelperComponent() = default;
    private:
        bool LateAwake() final;
        void OnLocalComplete() final;
    public:
        void GetLocation(std::string & address);
        void GetLocation(long long userId, std::string & address);
        bool OnAllot(long long userId, const std::string & service, const std::string & address);
        bool OnAllot(long long userId, const std::unordered_map<std::string, std::string> & infos);
    private:
        std::vector<std::string> mLocations;
        class InnerNetMessageComponent * mInnerComponent;
        std::unordered_map<std::string, int> mLocationWeights;
		std::unordered_map<std::string, std::unordered_set<std::string>> mEventMap;
    };
}


#endif //APP_FORWARDHELPERCOMPONENT_H
