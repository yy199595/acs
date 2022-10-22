//
// Created by zmhy0073 on 2022/10/20.
//

#ifndef APP_SUBPUBLISHCOMPONENT_H
#define APP_SUBPUBLISHCOMPONENT_H
#include"Component/Component.h"
namespace Sentry
{
	class SubPublishComponent : public Component, public IComplete
    {
	 public:
		SubPublishComponent() = default;
	 public:
		int Subscribe(const std::string & channel);
		int UnSubscribe(const std::string & channel);
		int Publish(const std::string & channel, const std::string & content);
	 protected:
		bool LateAwake() final;
		void OnLocalComplete() final;
	 private:
		std::vector<std::string> mLocations;
		class InnerNetMessageComponent * mInnerComponent;
	};
}


#endif //APP_SUBPUBLISHCOMPONENT_H
