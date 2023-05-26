//
// Created by MyPC on 2023/4/15.
//

#ifndef APP_REGISTRYCOMPONENT_H
#define APP_REGISTRYCOMPONENT_H
#include"Entity/Component/Component.h"
namespace Tendo
{
	class RedisTcpClient;
	class RegistryComponent : public Component
	{
	public:
		RegistryComponent() = default;
	public:
		void WaitRegister();
	private:
		bool LateAwake() final;
		bool RegisterServer();
	private:
		class Actor * mThisActor;
		class CoroutineComponent * mCorComponent;
	};
}

#endif //APP_REGISTRYCOMPONENT_H
