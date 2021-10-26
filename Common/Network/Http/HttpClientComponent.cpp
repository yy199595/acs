//
// Created by 64658 on 2021/8/5.
//
#include <Core/App.h>
#include <Thread/TaskThread.h>
#include "HttpClientComponent.h"
#include "Component/Scene/TaskPoolComponent.h"
#include <Util/StringHelper.h>
#include <Coroutine/CoroutineComponent.h>
#include <Network/Http/HttpLocalSession.h>
#include <Network/Http/HttpGetRequest.h>
#include <Network/Http/HttpDownLoadRequest.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpGetRequestTask.h>
#include <Network/Http/HttpDownloadRequestTask.h>
namespace Sentry
{
    SessionBase *HttpClientComponent::CreateSocket()
    {
        return new HttpRemoteSession(this);
    }

    bool HttpClientComponent::Awake()
    {
        this->mTaskComponent = this->GetComponent<TaskPoolComponent>();
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
        return true;
    }

	void HttpClientComponent::Start()
	{
		std::string json;
		long long t1 = TimeHelper::GetMilTimestamp();
        const std::string path = App::Get().GetWorkPath() + "download/";
		this->DownLoad("http://lrs-oss.whitewolvesx.com/app/default/boy.png", path);

        this->Get("http://timor.tech/api/holiday/year/2022", json);
		//SayNoDebugFatal(json);
		SayNoDebugError("time = " << (TimeHelper::GetMilTimestamp() - t1) / 1000.0f);
	}

    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
//        HttpGetRequest getRequest(this);
//        return getRequest.Get(url, json);

        HttpGetRequestTask httpGetRequestTask(url);
        if(!this->mTaskComponent->StartTask(&httpGetRequestTask))
        {
            return XCode::HttpTaskStarFail;
        }
        return httpGetRequestTask.Get(json);
    }

    XCode HttpClientComponent::DownLoad(const std::string &url, const std::string &path)
    {
		HttpDownloadRequestTask dowloadRequest(url);
		if (!this->mTaskComponent->StartTask(&dowloadRequest))
		{
			return XCode::HttpTaskStarFail;
		}
		return dowloadRequest.Download(path);
        HttpDownLoadRequest downLoadRequest(this);
        return downLoadRequest.DownLoad(url, path);
    }
}