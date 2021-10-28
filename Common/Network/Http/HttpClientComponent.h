
#pragma once
#include<Component/Component.h>
#include <Network/Http/HttpSessionBase.h>
namespace Sentry
{
    class HttpClientComponent : public Component, public SocketHandler<HttpSessionBase>
    {
    public:
        HttpClientComponent() = default;
        ~HttpClientComponent() = default;

    protected:
         SessionBase * CreateSocket() override;
    public:
        bool Awake() final;
        void Start() final;
    public:
        XCode Get(const std::string &url, std::string &json, int timeout = 5);
        XCode Post(const std::string & url,std::string & response, int timeout = 5);
        XCode DownLoad(const std::string & url, const std::string & path, int timeout =5);
        XCode DownLoad(const std::string & url, const std::string & path,const std::string & name, int timeout =5);
        XCode Post(const std::string & url, const std::string & data, std::string & response, int timeout = 5);
        XCode Post(const std::string & url, const std::unordered_map<std::string, std::string> & data, std::string & response, int timeout = 5);

    private:
        class TaskPoolComponent *mTaskComponent;
        class CoroutineComponent *mCorComponent;
    };
}