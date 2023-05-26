//
// Created by MyPC on 2023/4/15.
//

#include"RegistryComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Config/CodeConfig.h"
#include "XCode/XCode.h"
#include "Message/s2s/registry.pb.h"

namespace Tendo
{
	bool RegistryComponent::LateAwake()
	{
		this->mCorComponent = this->mApp->GetCoroutine();
		LOG_CHECK_RET_FALSE(this->mApp->ActorMgr()->GetServer(0));
		LOG_CHECK_RET_FALSE(this->mThisActor = this->mUnit->Cast<Actor>());
		return true;
	}

	bool RegistryComponent::RegisterServer()
	{
		const std::string func("Registry.Register");
		Server * registry = this->mApp->ActorMgr()->GetServer(0);

		registry::actor request;
		{
			request.set_name(this->mThisActor->Name());
			request.set_actor_id(this->mThisActor->GetActorId());
			this->mThisActor->OnRegister(*request.mutable_name());
		}
		return registry->Call(func, request) == XCode::Successful;
	}

	void RegistryComponent::WaitRegister()
	{
		while(!this->RegisterServer())
		{
			LOG_ERROR("register " << this->mThisActor->Name()
				<< " failure actor id=" << this->mThisActor->GetActorId());
			this->mApp->GetCoroutine()->Sleep(1000);
		}
	}
}