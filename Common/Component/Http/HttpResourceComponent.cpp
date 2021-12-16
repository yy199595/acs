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
        const ServerPath &serverPath = App::Get().GetServerPath();
        this->mDownloadPath = serverPath.GetDownloadPath();
        this->Add("Files", &HttpResourceComponent::Files, this);
        this->Add("Download", &HttpResourceComponent::Download, this);
        return true;
    }

    bool HttpResourceComponent::OnLoadConfig()
    {
        this->mFileMd5Map.clear();
        std::vector<std::string> files;
        if (!DirectoryHelper::GetFilePaths(this->mDownloadPath, files))
        {
            LOG_ERROR("not find dir : " << mDownloadPath);
            return false;
        }
        for (const std::string &path: files)
        {
            std::ifstream fs(path, std::ios::in);
            if (!fs.is_open())
            {
                LOG_ERROR("not open file : " << path);
                return false;
            }
            MD5 md5(fs);
            const std::string dir = path.substr(mDownloadPath.size() +1);
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

	HttpStatus HttpResourceComponent::Download(HttpRespSession *handler)
    {
        auto getRequest = handler->GetReuqestHandler();
        auto content = dynamic_cast<HttpReadStringContent*>(getRequest->GetContent());
        const std::string & fileName = content->GetContent();
        auto iter = this->mFileMd5Map.find(fileName);
        if(iter == this->mFileMd5Map.end())
        {
            return HttpStatus::NOT_FOUND;
        }
        const std::string dir = this->mDownloadPath + fileName;
        getRequest->SetResponseContent(new HttpWriteFileContent(dir));
        return HttpStatus::OK;
    }

    bool HttpResourceComponent::LateAwake()
    {
        return true;
    }
}