//
// Created by zmhy0073 on 2022/6/21.
//

#ifndef SERVER_HTTPSERVICECOMPONENT_H
#define SERVER_HTTPSERVICECOMPONENT_H
#include<unordered_set>
#include"HttpListenComponent.h"
namespace Http
{
    class Request;
    class DataResponse;
}

namespace Http
{
    class StaticSource
    {
    public:
        std::string mPath;
        std::string mType;
    };
}

namespace Sentry
{
    class HttpMethodConfig;
    class HttpHandlerClient;
	class HttpWebComponent : public HttpListenComponent, public IServerRecord, public IDestroy
    {
    public:
        HttpWebComponent();
        ~HttpWebComponent() = default;
    public:
		void AddStaticDir(const std::string & dir);
		unsigned int GetWaitCount() const { return this->mWaitCount; }
    private:
        bool LateAwake() final;
		void OnDestroy() final;
		void OnRecord(Json::Writer &document) final;
        bool OnDelClient(const std::string& address) final;
		void OnRequest(std::shared_ptr<Http::Request> request) final;
		void Invoke(const HttpMethodConfig* config, const std::shared_ptr<Http::Request>& request);
    private:
        unsigned int mSumCount;
        unsigned int mWaitCount;
        class AsyncMgrComponent * mTaskComponent;
        std::unordered_map<std::string, unsigned int> mTasks;
        std::unordered_map<std::string, std::string> mTypeContent;
		std::unordered_map<std::string, Http::StaticSource> mStaticSourceDir;
	};
}


#endif //SERVER_HTTPSERVICECOMPONENT_H
