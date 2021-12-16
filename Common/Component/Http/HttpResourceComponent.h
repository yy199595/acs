//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPRESOURCECOMPONENT_H
#define GAMEKEEPER_HTTPRESOURCECOMPONENT_H
#include"HttpServiceComponent.h"
namespace GameKeeper
{
    class HttpRespSession;
    class HttpResourceComponent : public HttpServiceComponent, public ILoadConfig
    {
    public:
        HttpResourceComponent() = default;
        ~HttpResourceComponent() override = default;
    private:
		XCode Files(RapidJsonWriter & response);
		HttpStatus Download(HttpRespSession * remoteSession);
    protected:
        bool Awake() final;
        bool LateAwake() final;
        bool OnLoadConfig() final;
    private:
        std::string mDownloadPath;
        std::unordered_map<std::string, std::string> mFileMd5Map;
    };
}

#endif //GAMEKEEPER_HTTPRESOURCECOMPONENT_H
