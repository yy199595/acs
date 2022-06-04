//
// Created by yjz on 2022/6/5.
//

#include"App/App.h"
#include"HttpSourceService.h"
#include"Util/DirectoryHelper.h"
namespace Sentry
{
	bool HttpSourceService::OnStartService(HttpServiceRegister& serviceRegister)
	{
		serviceRegister.Bind("Files", &HttpSourceService::Files);
		serviceRegister.Bind("Download", &HttpSourceService::Download);
		return true;
	}

	XCode HttpSourceService::Files(const HttpHandlerRequest& request, HttpHandlerResponse& response)
	{
		std::string path;
		std::vector<std::string> files;
		this->GetConfig().GetPath("source", path);
		const std::string & workPath = this->GetConfig().GetWorkPath();
		if(Helper::Directory::GetFilePaths(path, files))
		{
			for(const std::string & file : files)
			{
				response.WriteString(file.substr(workPath.size()) + "\n");
			}
		}
		return XCode::Successful;
	}

	XCode HttpSourceService::Download(const HttpHandlerRequest& request, HttpHandlerResponse& response)
	{
		std::fstream * fs = new std::fstream ();
		const std::string & workPath = this->GetConfig().GetWorkPath();
		fs->open(workPath + request.GetContent(), std::ios::binary | std::ios::in | std::ios::out);
		if(fs->is_open())
		{
			response.WriteFile(fs);
		}
		return XCode::Successful;
	}
}