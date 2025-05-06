//
// Created by 64658 on 2025/4/10.
//

#ifndef APP_PUBSUBCOMPONENT_H
#define APP_PUBSUBCOMPONENT_H
#include "Entity/Component/Component.h"
namespace acs
{
	class PubSubComponent : public Component
	{
	public:
		PubSubComponent();
	private:
		bool LateAwake() final;
	public:
		int Subscribe(const std::string & channel);
		int UnSubscribe(const std::string & channel);
		int Publish(const std::string & channel, const std::string & message);
		int Publish(const std::string & channel, const json::w::Document & message);
	private:
		std::string mNodeName;
		class NodeComponent * mNode;
	};
}


#endif //APP_PUBSUBCOMPONENT_H
