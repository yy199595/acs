//
// Created by zmhy0073 on 2021/11/1.
//

#include "HttpRequest.h"
#include <Define/CommonDef.h>
#include <Core/App.h>
#include <Network/Http/HttpLocalsession.h>
#include <Network/NetworkHelper.h>
#include <Scene/TaskPoolComponent.h>
namespace GameKeeper
{
    HttpRequest::HttpRequest(HttpClientComponent *component)
            : mHttpComponent(component)
    {
		this->mLocalSession = nullptr;
    }

	HttpRequest::~HttpRequest()
	{
		delete this->mLocalSession;
	}

    XCode HttpRequest::StartHttpRequest(const std::string & url)
    {
        if (!NetworkHelper::ParseHttpUrl(url, this->mHost, this->mPort, this->mPath))
        {
            return XCode::HttpUrlParseError;
        }
		
        auto taskComponent = App::Get().GetComponent<TaskPoolComponent>();
		NetWorkThread & nThread = taskComponent->GetNetThread();

        SocketProxy * socketProxy = new SocketProxy(nThread, "HttpGet");
		this->mLocalSession = new HttpLocalSession(this->mHttpComponent, this);
        this->mLocalSession->StartConnectHost(this->mHost, this->mPort, socketProxy);

        App::Get().GetCorComponent()->YieldReturn(this->mCorId);

        if (this->mCode != XCode::Successful)
        {
            return this->mCode;
        }
        if (this->mHttpCode != 200)
        {
            return XCode::HttpResponseError;
        }
        return XCode::Successful;
    }

	void HttpRequest::OnWriterAfter(XCode code)
	{
		this->mLocalSession->StartReceiveHeard();
	}

	void HttpRequest::OnReceiveBodyAfter(XCode code)
    {
        this->mCode = code;
        CoroutineComponent *corComponent = App::Get().GetCorComponent();
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.AddMainTask(&CoroutineComponent::Resume, corComponent, this->mCorId);
    }

    void HttpRequest::OnReceiveHeardAfter(XCode code)
    {
        if(code != XCode::Successful)
        {
            this->OnReceiveBodyAfter(code);
        }
		GKDebugWarning(this->PrintHeard());
    }

    bool HttpRequest::OnReceiveHeard(asio::streambuf &streamBuf)
    {
        std::istream is(&streamBuf);
        is >> this->mVersion >> this->mHttpCode >> this->mError;
#ifdef __DEBUG__
        GKDebugWarning(this->mVersion << " " << this->mHttpCode << " " << this->mError);
#endif
        this->ParseHeard(streamBuf);
        return true;
    }
}