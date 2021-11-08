//
// Created by zmhy0073 on 2021/11/2.
//
#include "HttpResourceComponent.h"
#include <Core/App.h>
#include <Util/DirectoryHelper.h>
#include <Util/MD5.h>
#include <Network/Http/Response/HttpGettHandler.h>
#include<Network/Http/Content/HttpWriteContent.h>
namespace GameKeeper
{
    bool HttpResourceComponent::Awake()
    {
        char buffer[1024] = {0};
        std::vector<std::string> files;
        this->Add("Files", &HttpResourceComponent::Files, this);
        this->Add("Download", &HttpResourceComponent::Download, this);

        const std::string & downloadPath = App::Get().GetDownloadPath();
        if (!DirectoryHelper::GetFilePaths(downloadPath, files))
        {
            GKDebugError("not find dir : " << downloadPath);
            return false;
        }
        for (const std::string &path: files)
        {
            std::ifstream fs(path, std::ios::in);
            if (!fs.is_open())
            {
                GKDebugError("not open file : " << path);
                return false;
            }
            MD5 md5(fs);
            const std::string dir = path.substr(downloadPath.size() +1);
            this->mFileMd5Map.insert(std::make_pair(dir, md5.toString()));
        }
        return true;
    }

	HttpStatus HttpResourceComponent::Files(HttpRequestHandler * handler)
    {
        auto getRequest = dynamic_cast<HttpGettHandler *>(handler);
        if (getRequest == nullptr)
        {
            return HttpStatus::BAD_REQUEST;
        }
        auto jsonContent = new HttpJsonContent();
        auto iter = this->mFileMd5Map.begin();
        for(;iter != this->mFileMd5Map.end();iter++)
        {
            const std::string & path = iter->first;
            const std::string & md5 = iter->second;
            jsonContent->Add(path.c_str(), md5);
        }
        //handler->SetContent(jsonContent);
        return HttpStatus::OK;
    }

	HttpStatus HttpResourceComponent::Download(HttpRequestHandler *handler)
    {
        auto getRequest = dynamic_cast<HttpGettHandler *>(handler);
        if (getRequest == nullptr)
        {
            return HttpStatus::BAD_REQUEST;
        }
//        const std::string & fileName = getRequest->GetParameter();
//        auto iter = this->mFileMd5Map.find(fileName);
//        if(iter == this->mFileMd5Map.end())
//        {
//            return HttpStatus::NOT_FOUND;
//        }
//        const std::string dir = App::Get().GetDownloadPath() + fileName;
        //getRequest->SetContent(new HttpWriteFileContent(dir));
        return HttpStatus::OK;
    }
}