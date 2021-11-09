//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPRESOURCECOMPONENT_H
#define GAMEKEEPER_HTTPRESOURCECOMPONENT_H
#include"HttpServiceComponent.h"
namespace GameKeeper
{
    class HttpRemoteSession;
    class HttpResourceComponent : public HttpServiceComponent
    {
    public:
        HttpResourceComponent() = default;
        ~HttpResourceComponent() override = default;
    private:
		XCode Files(RapidJsonWriter & response);
		HttpStatus Download(HttpRemoteSession * remoteSession);
    protected:
        bool Awake() override;
    private:
        std::unordered_map<std::string, std::string> mFileMd5Map;
    };
}

#endif //GAMEKEEPER_HTTPRESOURCECOMPONENT_H