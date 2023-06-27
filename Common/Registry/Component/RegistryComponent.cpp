//
// Created by MyPC on 2023/4/15.
//

#include"RegistryComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Config/CodeConfig.h"
#include "XCode/XCode.h"
#include"Message/com/com.pb.h"
#include"Message/s2s/registry.pb.h"
namespace Tendo
{
	bool RegistryComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mApp->ActorMgr()->GetServer(0));
		return true;
	}

	bool RegistryComponent::RegisterServer() const
	{
		const std::string func("ServerRegistry.Add");
		ServerActor * registryActor = this->mApp->ActorMgr()->GetServer(0);
		{
			com::type::json request;
			this->mApp->OnRegister(request.mutable_json());
			const int code = registryActor->Call(func, request);
			if (code != XCode::Successful)
			{
				LOG_ERROR("registry code = " << CodeConfig::Inst()->GetDesc(code));
				return false;
			}
			LOG_INFO("register to center successful");
		}
		return true;
	}

	void RegistryComponent::Complete()
	{
		while(!this->RegisterServer())
		{
			LOG_ERROR("register " << this->mApp->Name()
				<< " failure actor id = " << this->mApp->GetActorId());
			this->mApp->GetCoroutine()->Sleep(5000);
		}
		std::vector<std::string> watchs;
		const std::string func("ServerRegistry.Query");
		std::shared_ptr<registry::query::response> response
			= std::make_shared<registry::query::response>();
		ActorComponent * actorComponent = this->mApp->ActorMgr();
		ServerActor * registryActor = this->mApp->ActorMgr()->GetServer(0);
		{
			this->mApp->GetWatch(watchs);
			registry::query::request request;
			for (const std::string& name: watchs)
			{
				request.set_name(name);
				int code = registryActor->Call(func, request, response);
				if(code != XCode::Successful)
				{
					LOG_ERROR("query [" << name << "] code = "
							<< CodeConfig::Inst()->GetDesc(code));
				}
				for(const std::string & json : response->actors())
				{
					actorComponent->AddServer(json);
				}
				if(actorComponent->Random(name) == nullptr)
				{
					LOG_WARN("wait [" << name << "] start ...");
				}
			}
		}
	}
}