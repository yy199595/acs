
#pragma once
#include<Component/Component.h>
namespace GameKeeper
{
	class HttpSessionBase;
    class HttpReadContent;
    class HttpWriteContent;
    class HttpRemoteSession;
    class HttpServiceMethod;
    class HttpRequestHandler;	
	class HttpLocalSession;

    class HttpComponent : public Component, public ISocketListen
    {
    public:
        HttpComponent() = default;

        ~HttpComponent() final = default;
    public:
        bool Awake() final;

        void Start() final;

    public:
        XCode Get(const std::string &url, std::string &json, int timeout = 5);

        XCode Post(const std::string &url, const std::string &data, std::string &response, int timeout = 5);

        XCode Post(const std::string &url, HttpWriteContent &content, HttpReadContent &response, int timeout);

    public:

		HttpLocalSession * NewLocalSession();

		HttpRemoteSession * NewRemoteSession();

        void OnListen(SocketProxy *socket) final;

		void OnRequest(HttpRemoteSession * remoteSession);

        HttpServiceMethod * GetHttpMethod(const std::string & service, const std::string & method);

    private:
        void Invoke(HttpRemoteSession *remoteRequest);

    public:
        HttpLocalSession * CreateLocalSession();
        HttpRemoteSession * CreateRemoteSession();
        void DeleteSession(HttpLocalSession * session);
        void DeleteSession(HttpRemoteSession * session);
    private:
        class CoroutineComponent *mCorComponent;
		class TaskPoolComponent * mTaskComponent;
        std::queue<HttpLocalSession *> mLocalSessionPool;
        std::queue<HttpRemoteSession *> mRemoteSessionPool;
		std::unordered_map<long long, HttpSessionBase *> mSessionMap;
    };
}