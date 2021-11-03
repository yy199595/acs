//
// Created by zmhy0073 on 2021/11/2.
//
#include "HttpDownloadService.h"
#include <Core/App.h>
#include <Util/DirectoryHelper.h>
#include <Network/Http/Response/HttpRemoteGetRequestHandler.h>
namespace GameKeeper
{
    bool HttpDownloadService::Awake()
    {
		this->Add("Files", &HttpDownloadService::Files, this);
        this->Add("Download", &HttpDownloadService::Download, this);
        return true;
    }

	HttpStatus HttpDownloadService::Files(HttpRemoteRequestHandler * handler)
    {
        auto getRequest = dynamic_cast<HttpRemoteGetRequestHandler *>(handler);
        if (getRequest == nullptr)
        {
            return HttpStatus::BAD_REQUEST;
        }

        std::vector<std::string> files;
        const std::string &dir = App::Get().GetDownloadPath();
        const std::string &format = getRequest->GetParamater();
        if (!DirectoryHelper::GetFilePaths(dir, format, files))
        {
            return HttpStatus::NOT_FOUND;
        }

        for (auto & file : files)
        {
            file = file.substr(dir.size() + 1);
        }

        auto jsonContent = new HttpJsonContent();
        jsonContent->AddParameter("files", files);
        handler->SetContent(jsonContent);
        return HttpStatus::OK;
    }

	HttpStatus HttpDownloadService::Download(HttpRemoteRequestHandler *handler)
    {
        auto getRequest = dynamic_cast<HttpRemoteGetRequestHandler *>(handler);
        if (getRequest == nullptr)
        {
            return HttpStatus::BAD_REQUEST;
        }
      
		const std::string & fileName = getRequest->GetParamater();
        const std::string dir = App::Get().GetDownloadPath() + fileName;
        if (!DirectoryHelper::DirectorIsExist(dir))
        {
            return HttpStatus::NOT_FOUND;
        }
        getRequest->SetContent(new HttpWriteFileContent(dir));
        return HttpStatus::OK;
    }
}