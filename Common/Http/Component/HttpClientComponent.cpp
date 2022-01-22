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
#include"Util/FileHelper.h"
#include"Service/HttpService.h"
namespace Sentry
{

    bool HttpClientComponent::Awake()
    {
        std::string path;
        this->mCorComponent = nullptr;
        rapidjson::Document jsonDocument;
        const ServerConfig &config = App::Get().GetConfig();
        LOG_CHECK_RET_FALSE(config.GetValue("path", "http", path));
        LOG_CHECK_RET_FALSE(Helper::File::ReadJsonFile(path, jsonDocument));

        auto iter = jsonDocument.MemberBegin();
        for (; iter != jsonDocument.MemberEnd(); iter++)
        {
            HttpConfig * httpConfig = new HttpConfig();
            httpConfig->mUrl = iter->name.GetString();
            const rapidjson::Value & jsonValue = iter->value;
            httpConfig->mType = jsonValue["Type"].GetString();
            httpConfig->mMethodName = jsonValue["Method"].GetString();
            httpConfig->mComponent = jsonValue["Component"].GetString();
            this->mHttpConfigMap.emplace(httpConfig->mUrl, httpConfig);
        }
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

            auto response = httpAsyncClient->Post("http://127.0.0.1:80/logic/account/login", json);
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

        const std::string & url = httpRequestData->GetUrl();
        auto iter = this->mHttpConfigMap.find(url);
        if(iter == this->mHttpConfigMap.end())
        {
            httpClient->SendResponse(HttpStatus::NOT_FOUND);
            return;
        }
        const std::string & content = httpRequestData->GetContent();
        std::shared_ptr<RapidJsonReader> jsonReader(new RapidJsonReader());
        if(!jsonReader->TryParse(content))
        {
            httpClient->SendResponse(HttpStatus::BAD_REQUEST);
            return;
        }
        HttpConfig * httpConfig = iter->second;
        HttpService * httpService = this->GetComponent<HttpService>(httpConfig->mComponent);
        if(httpService == nullptr)
        {
            httpClient->SendResponse(HttpStatus::NOT_FOUND);
            return;
        }
        const std::string & method = httpConfig->mMethodName;
        auto response = httpService->Invoke(method,jsonReader);

        std::string json;
        response->WriterToStream(json);
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