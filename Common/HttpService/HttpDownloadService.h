//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPDOWNLOADSERVICE_H
#define GAMEKEEPER_HTTPDOWNLOADSERVICE_H
#include"HttpServiceComponent.h"
namespace GameKeeper
{
    class HttpDownloadService : public HttpServiceComponent
    {
    public:
        HttpDownloadService() = default;
        ~HttpDownloadService() override = default;
    private:
		HttpStatus Files(HttpRemoteRequest * handler);
        HttpStatus Download(HttpRemoteRequest * handler);
    protected:
        bool Awake() override;
    };
}

#endif //GAMEKEEPER_HTTPDOWNLOADSERVICE_H
