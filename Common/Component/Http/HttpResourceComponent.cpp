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
        this->Add("Files", &HttpResourceComponent::Files, this);
        this->Add("Download", &HttpResourceComponent::Download, this);

        return true;
    }

    bool HttpResourceComponent::OnLoadConfig()
    {
        this->mFileMd5Map.clear();
        std::vector<std::string> files;
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

	XCode HttpResourceComponent::Files(RapidJsonWriter & jsonResponse)
    {
        auto iter = this->mFileMd5Map.begin();
        for(;iter != this->mFileMd5Map.end();iter++)
        {
            const std::string & path = iter->first;
            const std::string & md5 = iter->second;
            jsonResponse.Add(path.c_str(), md5);
        }
        return XCode::Successful;
    }

	HttpStatus HttpResourceComponent::Download(HttpRemoteSession *handler)
    {
        auto getRequest = handler->GetReuqestHandler();
        auto content = dynamic_cast<HttpReadStringContent*>(getRequest->GetContent());
        const std::string & fileName = content->GetContent();
        auto iter = this->mFileMd5Map.find(fileName);
        if(iter == this->mFileMd5Map.end())
        {
            return HttpStatus::NOT_FOUND;
        }
        const std::string dir = App::Get().GetDownloadPath() + fileName;
        getRequest->SetResponseContent(new HttpWriteFileContent(dir));
        return HttpStatus::OK;
    }
}