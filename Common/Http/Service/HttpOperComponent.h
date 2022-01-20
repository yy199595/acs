//
// Created by zmhy0073 on 2021/11/11.
//

#ifndef GAMEKEEPER_HTTPOPERCOMPONENT_H
#define GAMEKEEPER_HTTPOPERCOMPONENT_H
#include "HttpServiceComponent.h"
namespace Sentry
{
    class HttpOperComponent : public HttpServiceComponent
    {
    public:
        HttpOperComponent() = default;
        ~HttpOperComponent() final = default;

    private:
        XCode Hotfix(RapidJsonWriter & response);
        XCode LoadConfig(RapidJsonWriter & response);

    private:
        XCode VerifyAccount(const RapidJsonReader & jsonData);
    protected:
        bool Awake() final;
        bool LateAwake() final;
    private:
        std::unordered_map<std::string, std::string> mOperAccountMap;
    };
}
#endif //GAMEKEEPER_HTTPOPERCOMPONENT_H
