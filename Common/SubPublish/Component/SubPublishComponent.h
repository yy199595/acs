//
// Created by zmhy0073 on 2022/10/20.
//

#ifndef APP_SUBPUBLISHCOMPONENT_H
#define APP_SUBPUBLISHCOMPONENT_H
#include"Client/Message.h"
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
		bool OnMessage(std::shared_ptr<Rpc::Packet> message);
		int Subscribe(std::unordered_set<std::string> & channels);
		int Publish(const std::string & channel, const std::string & content);
	 protected:
		bool LateAwake() final;
		void OnLocalComplete() final;
	 private:
		std::vector<std::string> mLocations;
		class InnerNetMessageComponent * mInnerComponent;
		std::unordered_map<std::string, std::unordered_set<std::string>> mChannels;
	};
}


#endif //APP_SUBPUBLISHCOMPONENT_H
