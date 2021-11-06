//
// Created by zmhy0073 on 2021/11/5.
//

#ifndef GAMEKEEPER_HTTPLOGINSERVICE_H
#define GAMEKEEPER_HTTPLOGINSERVICE_H
#include <HttpService/HttpServiceComponent.h>
namespace GameKeeper
{
    class HttpLoginService : public HttpServiceComponent
    {
    public:
        HttpLoginService() = default;
        ~HttpLoginService() final = default;

    protected:
        bool Awake() override;
    private:
        XCode Login(const RapidJsonReader & request, RapidJsonWriter & response);
    };
}
#endif //GAMEKEEPER_HTTPLOGINSERVICE_H
