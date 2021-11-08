//
// Created by zmhy0073 on 2021/11/5.
//
#include<Core/App.h>
#include "HttpLoginService.h"
#include<Util/DirectoryHelper.h>
#include<Util/MD5.h>
namespace GameKeeper
{
    bool HttpLoginService::Awake()
    {
        this->Add("Login", &HttpLoginService::Login, this);
        return true;
    }

    XCode HttpLoginService::Login(const RapidJsonReader &request, RapidJsonWriter &response)
    {
		std::vector<std::string> paths;
		const std::string & path = App::Get().GetDownloadPath();

		if (DirectoryHelper::GetFilePaths(path, paths))
		{
			response.StartArray("files");
			for (const std::string & dir : paths)
			{
				response.StartObject();
				std::ifstream fs(dir, std::ios::in);
                fs.seekg(0, std::ios_base::beg);
                size_t size  = fs.seekg(0, std::ios_base::end).tellg();


				MD5 md5(fs);
				response.Add("name", dir);
                response.Add("size", (unsigned int)size);
				response.Add("md5", md5.toString());
				response.EndObject();
			}
			response.EndArray();
		}
        std::string account;
        std::string password;
        if(!request.TryGetValue("account", account))
        {
            return XCode::Failure;
        }
        if(!request.TryGetValue("password", password))
        {
            return XCode::Failure;
        }
        GKDebugLog(account << "   " << password);
        return XCode::Successful;
    }
}