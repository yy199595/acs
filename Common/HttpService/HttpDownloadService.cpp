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
		this->Add("Files", &HttpDownloadService::Files, this);
        this->Add("Download", &HttpDownloadService::Download, this);
        return true;
    }

	HttpStatus HttpDownloadService::Files(HttpRemoteRequest * handler)
	{
		auto getRequest = dynamic_cast<HttpRemoteGetRequest *>(handler);
		if (getRequest == nullptr)
		{
			return HttpStatus::BAD_REQUEST;
		}

		std::stringstream streamBuf;
		std::vector<std::string> files;
		const std::string & dir = App::Get().GetDownloadPath();
		const std::string & format = getRequest->GetParamater();
		if (!DirectoryHelper::GetFilePaths(dir, format, files))
		{
			return HttpStatus::NOT_FOUND;
		}
		for (const std::string & file : files)
		{
			streamBuf << file.substr(dir.size() + 1) << "\n";
		}
		handler->SetContent(new HttpStringContent(streamBuf.str()));
		return HttpStatus::OK;
	}

	HttpStatus HttpDownloadService::Download(HttpRemoteRequest *handler)
    {
        auto getRequest = dynamic_cast<HttpRemoteGetRequest *>(handler);
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
        getRequest->SetContent(new HttpFileContent(dir));
        return HttpStatus::OK;
    }
}