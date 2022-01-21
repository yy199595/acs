//
// Created by 64658 on 2021/8/5.
//
#include "Object/App.h"
#include "Thread/TaskThread.h"
#include "HttpClientComponent.h"
#include "Method/HttpServiceMethod.h"
#include "Other/ProtoConfig.h"
#include"Other/ElapsedTimer.h"
#include"Scene/LoggerComponent.h"
#include"Scene/ThreadPoolComponent.h"
#include"Http/HttpRequestClient.h"
#include"Http/HttpHandlerClient.h"
namespace Sentry
{

    bool HttpClientComponent::Awake()
    {
        this->mCorComponent = nullptr;
        return true;
    }

    bool HttpClientComponent::LateAwake()
    {
        this->mCorComponent = App::Get().GetTaskComponent();
        this->mThreadComponent = this->GetComponent<ThreadPoolComponent>();

        std::string url1 = "http://v.juhe.cn/telecode/to_telecodes.php";
        this->mCorComponent->Start([this, url1]() {

        });
        return true;
    }

    void HttpClientComponent::OnLoadData()
    {
        std::string url = "http://langrens.oss-cn-shenzhen.aliyuncs.com/res/area/city-config.json";

        std::string json;
        RapidJsonWriter jsonWriter;
        jsonWriter.Add("End", 100);
        jsonWriter.Add("Start", 0);
        jsonWriter.Add("RankId", 301000);

        jsonWriter.WriterToStream(json);
        while(!json.empty())
        {
            ElapsedTimer elapsedTimer;
            NetWorkThread &thread = this->mThreadComponent->AllocateNetThread();
            std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(thread, "HttpRequest"));
            std::shared_ptr<HttpRequestClient> httpAsyncClient(new HttpRequestClient(socketProxy));

            auto response = httpAsyncClient->Post("http://dev-lrs-tt.whitewolvesx.com/auth-server/user/login/phone", json);
            if (response != nullptr && response->GetHttpCode() == HttpStatus::OK)
            {
                LOG_ERROR(response->GetContent(), " time = [", elapsedTimer.GetMs(), "ms]");
            }
        }
    }

    void HttpClientComponent::OnListen(std::shared_ptr<SocketProxy> socket)
    {
        std::shared_ptr<HttpHandlerClient> handlerClient(new HttpHandlerClient(socket));
        this->mCorComponent->Start(&HttpClientComponent::HandlerHttpData, this, handlerClient);
    }

    void HttpClientComponent::HandlerHttpData(std::shared_ptr<HttpHandlerClient> httpClient)
    {
        ElapsedTimer elapsedTimer;
        std::shared_ptr<HttpHandlerRequest> httpRequestData = httpClient->ReadHandlerContent();
        LOG_CHECK_RET(httpRequestData);
        LOG_WARN(httpRequestData->GetMethod(), "  ", httpRequestData->GetUrl(), "  ", httpRequestData->GetContent());
        //LOG_WARN(requestData->GetContent());

        std::string json;
        RapidJsonWriter jsonWriter;
        jsonWriter.Add("End", 100);
        jsonWriter.Add("Start", 0);
        jsonWriter.Add("RankId", 301000);

        jsonWriter.WriterToStream(json);

        if(httpClient->SendResponse(HttpStatus::OK, json))
        {
            LOG_INFO("http data response successful [", elapsedTimer.GetMs(), "ms]");
        }
    }

    XCode HttpClientComponent::Get(const std::string &url, int timeout)
    {
        return XCode::Successful;
    }


    XCode HttpClientComponent::Post(const std::string &url, const std::string & data, int timeout)
    {

        return XCode::Successful;
    }

    void HttpClientComponent::Invoke(HttpRespSession *remoteRequest)
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
}