//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPDOWNLOADSERVICE_H
#define GAMEKEEPER_HTTPDOWNLOADSERVICE_H
#include"HttpServiceComponent.h"
namespace GameKeeper
{
    class HttpRemoteRequestHandler;
    class HttpDownloadService : public HttpServiceComponent
    {
    public:
        HttpDownloadService() = default;
        ~HttpDownloadService() override = default;
    private:
		HttpStatus Files(HttpRemoteRequestHandler * handler);
        HttpStatus Download(HttpRemoteRequestHandler * handler);
    protected:
        bool Awake() override;
    private:
        std::unordered_map<std::string, std::string> mFileMd5Map;
    };
}

#endif //GAMEKEEPER_HTTPDOWNLOADSERVICE_H
