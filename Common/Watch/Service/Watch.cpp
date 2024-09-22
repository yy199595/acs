//
// Created by yy on 2024/7/5.
//

#include "Watch.h"
#include "Watch/Component/WatchComponent.h"
namespace acs
{
	Watch::Watch()
	{
		this->mWatch = nullptr;
	}

	bool Watch::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(Watch::List);
		BIND_COMMON_HTTP_METHOD(Watch::StartProcess);
		BIND_COMMON_HTTP_METHOD(Watch::CloseProcess);
		LOG_CHECK_RET_FALSE(this->mWatch = this->GetComponent<WatchComponent>());
		return true;
	}

	int Watch::List(const http::FromContent& request, json::w::Document& response)
	{
		this->mWatch->GetAllProcess(response);
		return XCode::Ok;
	}

	int Watch::StartProcess(const json::r::Document& request, json::w::Document& response)
	{
		std::string exe, args, name;
		LOG_ERROR_CHECK_ARGS(request.Get("exe", exe));
		LOG_ERROR_CHECK_ARGS(request.Get("args", args));
		LOG_ERROR_CHECK_ARGS(request.Get("name", name));

		if(!this->mWatch->StartProcess(name, exe, args))
		{
			return XCode::Failure;
		}
		return XCode::Ok;
	}

	int Watch::CloseProcess(const http::FromContent& request, json::w::Document& response)
	{
		int pid = 0;
		LOG_ERROR_CHECK_ARGS(request.Get("pid", pid));
		if(!this->mWatch->CloseProcess(pid))
		{
			return XCode::Failure;
		}
		return XCode::Ok;
	}
}