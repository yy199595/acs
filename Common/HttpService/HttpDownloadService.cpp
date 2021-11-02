//
// Created by zmhy0073 on 2021/11/2.
//
#include "HttpDownloadService.h"
#include <Core/App.h>
#include <Util/DirectoryHelper.h>
#include <Network/Http/Response/HttpRemoteGetRequest.h>
namespace GameKeeper
{
    bool HttpDownloadService::Awake()
    {
        this->Add("/App/HttpDownloadService/Download", &HttpDownloadService::Download, this);
        return true;
    }

    HttpStatus HttpDownloadService::Download(HttpRemoteRequest *handler)
    {
        auto getRequest = dynamic_cast<HttpRemoteGetRequest *>(handler);

        if (getRequest == nullptr)
        {
            return HttpStatus::BAD_REQUEST;
        }

        std::string fileName;
        if (!getRequest->GetParameter("file", fileName))
        {
            return HttpStatus::BAD_REQUEST;
        }

        const std::string dir = App::Get().GetConfigPath() + fileName;
        if (!DirectoryHelper::DirectorIsExist(dir))
        {
            return HttpStatus::NOT_FOUND;
        }
        getRequest->SetContent(new HttpFileContent(dir));
        return HttpStatus::OK;
    }
}