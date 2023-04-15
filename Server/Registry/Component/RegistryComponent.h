//
// Created by MyPC on 2023/4/15.
//

#ifndef APP_REGISTRYCOMPONENT_H
#define APP_REGISTRYCOMPONENT_H
#include"Entity/Component/Component.h"
namespace Tendo
{
	class RegistryComponent : public Component, public IComplete
	{
	public:
		RegistryComponent();
	public:
		bool Query(const std::string & server, bool async = true);
	private:
		bool LateAwake() final;
		void OnLocalComplete() final;
	private:
		std::string mAddress;
		class NodeMgrComponent * mNodeComponent;
		class RedisLuaComponent * mRedisComponent;
	};
}

#endif //APP_REGISTRYCOMPONENT_H
