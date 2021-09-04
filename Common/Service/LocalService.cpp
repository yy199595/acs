#include "LocalService.h"

#include <Core/App.h>
#include <Scene/SceneActionComponent.h>
#include <Coroutine/CoroutineComponent.h>
#include <Scene/SceneProtocolComponent.h>
namespace Sentry
{
    LocalService::LocalService()
    {
    }

	bool LocalService::AddMethod(ServiceMethod * method)
	{
		
	}

	bool LocalService::HasMethod(const std::string &method)
    {
		auto iter = this->mMethodMap.find(method);
		return iter != this->mMethodMap.end();
    }

	ServiceMethod * LocalService::GetMethod(const std::string &method)
	{
		auto iter = this->mMethodMap.find(method);
		return iter != this->mMethodMap.end() ? iter->second : nullptr;
	}
}