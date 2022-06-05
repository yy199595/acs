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
		this->GetConfig().GetPath("source", this->mSourcePath);
		serviceRegister.Bind("Files", &HttpSourceService::Files);
		serviceRegister.Bind("Download", &HttpSourceService::Download);
		return true;
	}

	XCode HttpSourceService::Files(const HttpHandlerRequest& request, HttpHandlerResponse& response)
	{
		std::vector<std::string> files;
		if(!Helper::Directory::GetFilePaths(this->mSourcePath, files))
		{
			return XCode::Failure;
		}
		for(const std::string & file : files)
		{
			response.WriteString(file.substr(this->mSourcePath.size()) + "\n");
		}
		return XCode::Successful;
	}

	XCode HttpSourceService::Download(const HttpHandlerRequest& request, HttpHandlerResponse& response)
	{
		std::fstream * fs = new std::fstream ();
		const string path = fmt::format(
			"{0}{1}", this->mSourcePath, request.GetContent());
		fs->open(path, std::ios::binary | std::ios::in);
		if(fs->is_open())
		{
			response.WriteFile(fs);
			return XCode::Successful;
		}
		delete fs;
		LOG_ERROR("open " << path << " error");
		return XCode::Failure;
	}
}