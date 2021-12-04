//
// Created by 64658 on 2021/8/5.
//
#include <Core/App.h>
#include <Thread/TaskThread.h>
#include "Component/Scene/HttpComponent.h"
#include <Coroutine/CoroutineComponent.h>
#include <Http/HttpRemoteSession.h>
#include <Http/Request/HttpGetRequest.h>
#include <Http/Request/HttpPostRequest.h>
#include <Method/HttpServiceMethod.h>
#include <Http/Content/HttpWriteContent.h>
#include <Http/Response/HttpPostHandler.h>
#include <Http/HttpServiceComponent.h>
#include <Other/ProtocolConfig.h>
#include<Http/HttpLocalsession.h>

#include"Core/App.h"
#include"Scene/LoggerComponent.h"
namespace GameKeeper
{

    bool HttpComponent::Awake()
    {
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
        return true;
    }

	void HttpComponent::Start()
    {
        std::string json;
        const std::string url = "http://langrensha01.oss-cn-shenzhen.aliyuncs.com/res/area/city-config.json";
        std::string data = "fid=0&key=f5c417a28abf995d7ce6312b29556fd9";
		/*db::UserAccountData userAccount;
		userAccount.set_account("646585122@11.com");
		userAccount.set_userid(TimeHelper::GetMilTimestamp());
		userAccount.set_passwd("1122334455");
		userAccount.set_phonenum(1371601995);
		userAccount.set_lastlogintime(TimeHelper::GetSecTimeStamp());
		XCode code = this->GetComponent<MysqlProxyComponent>()->Add(userAccount);

		LOG_ERROR("code = " << code);*/

        //XCode code = this->Get(url, json);

        //LOG_FATAL(code << "\n" << json.size());

//        HttpJsonContent jsonContent;
//		HttpReadStringContent stringContent;
//		jsonContent.Add("account", "646585122@qq.com");
//		jsonContent.Add("password", "11223344566");
//		while (true)
//		{
//			this->Post("http://127.0.0.1:80/App/HttpLoginService/Login", jsonContent, stringContent, 5);
//		}
       

		//LOG_INFO(json);
        //this->Get("http://lrs-oss.whitewolvesx.com/app/default/boy.png", json);
	
        //LOG_FATAL(StringHelper::FormatJson(json));
    }

    void HttpComponent::OnListen(SocketProxy *socket)
    {
        auto httpSession = this->CreateRemoteSession();
        if(httpSession != nullptr)
        {
            httpSession->Start(socket);
        }
    }

	void HttpComponent::OnRequest(HttpRemoteSession * remoteSession)
	{
        auto requestHandler = remoteSession->GetReuqestHandler();
        const std::string & method = requestHandler->GetMethod();
        const std::string & service = requestHandler->GetComponent();
        auto httpMethod = this->GetHttpMethod(service, method);

        if(httpMethod == nullptr)
        {		
            requestHandler->SetResponseCode(HttpStatus::NOT_FOUND);
			LOG_ERROR("not find http method " << service << "." << method);
        }
        else
        {
            requestHandler->SetResponseCode(httpMethod->Invoke(remoteSession));
        }
        remoteSession->StartSendHttpMessage();
	}

    HttpServiceMethod *HttpComponent::GetHttpMethod(const std::string &service, const std::string &method)
    {
        auto component = this->GetComponent<HttpServiceComponent>(service);
        if(component == nullptr)
        {
            return nullptr;
        }
        return component->GetMethod(method);
    }

    XCode HttpComponent::Get(const std::string &url, std::string &json, int timeout)
    {
        HttpReadStringContent content(json);
        HttpLocalSession httpLocalSession(this);
#ifdef __DEBUG__
        long long t1 = TimeHelper::GetMilTimestamp();
        XCode code = httpLocalSession.Get(url, content);
        LOG_INFO("get " << url << " use time [" << (TimeHelper::GetMilTimestamp() - t1) / 1000.f << "s]");
        return code;
#else
        return httpLocalSession.Get(url, content);
#endif
    }


    XCode HttpComponent::Post(const std::string &url, const std::string & request, std::string &response, int timeout)
    {
        HttpWriteStringContent writeContent(request);
        HttpReadStringContent readContent(response);
        HttpLocalSession httpLocalSession(this);
        return this->Post(url, writeContent, readContent, timeout);
    }

    XCode HttpComponent::Post(const std::string & url, HttpWriteContent & content, HttpReadContent & response, int timeout)
    {
        HttpLocalSession httpLocalSession(this);
#ifdef __DEBUG__
        long long t1 = TimeHelper::GetMilTimestamp();
        XCode code = httpLocalSession.Post(url, content, response);
        LOG_INFO("post " << url << " use time [" << (TimeHelper::GetMilTimestamp() - t1) / 1000.f << "s]");
        return code;
#else
        return httpLocalSession.Post(url, content, response);
#endif
    }

    void HttpComponent::Invoke(HttpRemoteSession *remoteRequest)
    {			
//		 HttpRequestHandler * requestHandler = remoteRequest->GetReuqestHandler();
//		 const HttpServiceConfig * httpConfig = requestHandler->GetHttpConfig();
//		 if (requestHandler != nullptr)
//		 {
//			 const std::string & method = httpConfig->Method;
//			 const std::string & service = httpConfig->Service;
//			 auto httpMethod = this->GetHttpMethod(service, method);
//			 requestHandler->SetResponseCode(httpMethod->OnResponse(remoteRequest));
//		 }
    }

    HttpLocalSession *HttpComponent::CreateLocalSession()
    {
        HttpLocalSession * httpLocalSession = nullptr;
        if(!this->mLocalSessionPool.empty())
        {
            httpLocalSession = this->mLocalSessionPool.front();
            this->mLocalSessionPool.pop();
            return  httpLocalSession;
        }
        return new HttpLocalSession(this);
    }

    HttpRemoteSession *HttpComponent::CreateRemoteSession()
    {
        HttpRemoteSession * remoteSession = nullptr;
        if(!this->mRemoteSessionPool.empty())
        {
            remoteSession = this->mRemoteSessionPool.front();
            this->mRemoteSessionPool.pop();
            return remoteSession;
        }
        return new HttpRemoteSession(this);
    }

    void HttpComponent::DeleteSession(HttpLocalSession *session)
    {
        if(this->mLocalSessionPool.size() >= 10)
        {
            delete session;
            return;
        }
        session->Clear();
        this->mLocalSessionPool.push(session);
    }

    void HttpComponent::DeleteSession(HttpRemoteSession *session)
    {
        if(this->mRemoteSessionPool.size() >= 10)
        {
            delete session;
            return;
        }
        session->Clear();
        this->mRemoteSessionPool.push(session);
    }

}