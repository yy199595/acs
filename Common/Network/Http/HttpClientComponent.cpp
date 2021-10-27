//
// Created by 64658 on 2021/8/5.
//
#include <Core/App.h>
#include <Thread/TaskThread.h>
#include "HttpClientComponent.h"
#include "Component/Scene/TaskPoolComponent.h"
#include <Util/StringHelper.h>
#include <Coroutine/CoroutineComponent.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/HttpGetRequestTask.h>
#include <Network/Http/HttpPostRequestTask.h>
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
        const std::string name = "新闻.json";
        const std::string path = App::Get().GetWorkPath() + "download/";
        const std::string url = R"(http:\/\/dfzximg02.dftoutiao.com\/news\/20211022\/20211022133828_1291a0e2ecd603a257e94b55673e5738_1_mwpm_03201609.jpeg)";
		this->DownLoad(url, path);

        //std::string data = "fid=0&key=f5c417a28abf995d7ce6312b29556fd9";
        //this->Post("http://apis.juhe.cn/xzqh/query?fid=0&key=f5c417a28abf995d7ce6312b29556fd9", json);

        SayNoDebugFatal(json);
		SayNoDebugError("time = " << (TimeHelper::GetMilTimestamp() - t1) / 1000.0f << "s");
	}

    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
        HttpGetRequestTask httpGetRequestTask(url);
        if(!this->mTaskComponent->StartTask(&httpGetRequestTask))
        {
            return XCode::HttpTaskStarFail;
        }
        return httpGetRequestTask.Get(json);
    }

    XCode HttpClientComponent::DownLoad(const std::string &url, const std::string &path, int timeout)
    {
		HttpDownloadRequestTask dowloadRequest(url);
		if (!this->mTaskComponent->StartTask(&dowloadRequest))
		{
			return XCode::HttpTaskStarFail;
		}
		return dowloadRequest.Download(path);
    }

    XCode HttpClientComponent::DownLoad(const std::string &url, const std::string &path, const std::string &name, int timeout)
    {
        HttpDownloadRequestTask dowloadRequest(url);
        if (!this->mTaskComponent->StartTask(&dowloadRequest))
        {
            return XCode::HttpTaskStarFail;
        }
        return dowloadRequest.Download(path, name);
    }

    XCode HttpClientComponent::Post(const std::string &url, const std::string & data, std::string &response, int timeout)
    {
        HttpPostRequestTask postRequestTask(url);
        if(!this->mTaskComponent->StartTask(&postRequestTask))
        {
            return XCode::HttpTaskStarFail;
        }
        return postRequestTask.Post(data, response);
    }

    XCode HttpClientComponent::Post(const std::string &url, std::string &response, int timeout)
    {
        const size_t pos = url.find('?');
        if (pos == std::string::npos)
        {
            return XCode::HttpUrlParseError;
        }
        std::string heard = url.substr(0, pos);
        std::string data = url.substr(pos + 1, url.size() - pos);
        return this->Post(heard, data, response);
    }

    XCode HttpClientComponent::Post(const std::string &url, const std::unordered_map<std::string, std::string> &data,
                                    std::string &response, int timeout)
    {
        if(data.size() == 0)
        {
            return XCode::HttpUrlParseError;
        }
        std::string parameter;
        for(auto iter = data.begin(); iter != data.end(); iter++)
        {
            if(iter != data.begin())
            {
                parameter.append("&");
            }
            const std::string & key = iter->first;
            const std::string & val = iter->second;
            parameter.append(key + '=' + val);
        }
        return this->Post(url, parameter, response);
    }

}