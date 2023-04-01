//
// Created by zmhy0073 on 2022/6/21.
//

#include"HttpWebComponent.h"
#include"Entity/App/App.h"
#include"Server/Config/CodeConfig.h"
#include"Server/Config/ServiceConfig.h"
#include"Http/Client/HttpHandlerClient.h"
#include"Http/Service/HttpService.h"
#include"Util/File/DirectoryHelper.h"
#include"Util/File/FileHelper.h"
#include"Server/Component/ThreadComponent.h"
namespace Sentry
{
    HttpWebComponent::HttpWebComponent()
        : mSumCount(0), mWaitCount(0),
          mTaskComponent(nullptr)
    {
        this->mTypeContent["js"] = Http::ContentName::JS;
        this->mTypeContent["css"] = Http::ContentName::CSS;
        this->mTypeContent["json"] = Http::ContentName::JSON;
        this->mTypeContent["html"] = Http::ContentName::HTML;
        this->mTypeContent["txt"] = Http::ContentName::STRING;
		this->mTypeContent["mp4"] = Http::ContentName::MP4;
        this->mTypeContent["ico"] = Http::ContentName::ICO;
        this->mTypeContent["png"] = Http::ContentName::PNG;
        this->mTypeContent["gif"] = Http::ContentName::GIF;
        this->mTypeContent["jpeg"] = Http::ContentName::JPEG;
    }

    bool HttpWebComponent::LateAwake()
    {
		std::string dir;
		this->mWaitCount = 0;
		if(ServerConfig::Inst()->GetPath("html", dir))
		{
			this->AddStaticDir(dir);
		}
        std::vector<HttpService *> httpServices;
        this->mTaskComponent = this->mApp->GetTaskComponent();
        return this->mApp->GetComponents(httpServices) && this->StartListen("http");
    }

	void HttpWebComponent::AddStaticDir(const std::string& dir)
	{
		std::vector<std::string> files;
		Helper::Directory::GetFilePaths(dir, files);
		for(auto iter = files.begin(); iter != files.end(); iter++)
		{
            std::string type;
			Http::StaticSource source;
			const std::string & fullPath = *iter;
			const std::string path = fullPath.substr(dir.size());
            if (Helper::File::GetFileType(fullPath, type))
            {
                auto iter1 = this->mTypeContent.find(type);
                if (iter1 != this->mTypeContent.end())
                {
                    source.mPath = fullPath;
                    source.mType = iter1->second;
                }
				else
				{
					source.mPath = fullPath;
					source.mType = Http::ContentName::Bin;
				}
				this->mStaticSourceDir.emplace(path, source);
			}
		}
	}

    void HttpWebComponent::OnRequest(std::shared_ptr<Http::Request> request)
    {
		const std::string & address = request->From();
        const HttpMethodConfig *httpConfig = HttpConfig::Inst()->GetMethodConfig(request->Path());
        if (httpConfig == nullptr)
		{
			const std::string& path = request->Path();
			auto iter = this->mStaticSourceDir.find(path);
			if (iter != this->mStaticSourceDir.end())
			{
				std::string content;
				std::ifstream* fs = new std::ifstream();
				Http::StaticSource& staticSource = iter->second;
				fs->open(staticSource.mPath, std::ios::in | std::ios::binary);
				if (!fs->is_open())
				{
					delete fs;
					this->Send(address, HttpStatus::NOT_FOUND);
					return;
				}
				this->Send(address, staticSource.mType, fs);
				return;
			}
			this->Send(address, HttpStatus::NOT_FOUND);
			LOG_ERROR("[" << address << "] <<" << request->Path() << ">>" << HttpStatusToString(HttpStatus::NOT_FOUND));
			return;
		}

        if (!httpConfig->Type.empty() && httpConfig->Type != request->Method())
        {			
			//this->ClosetHttpClient(address);
            this->Send(address, HttpStatus::METHOD_NOT_ALLOWED);
            LOG_ERROR("[" << address << "] <<" << request->Url() << ">>" << HttpStatusToString(HttpStatus::METHOD_NOT_ALLOWED));
            return;
        }

        if (!httpConfig->IsAsync)
        {
            this->Invoke(httpConfig, request);
            return;
        }
        this->mTaskComponent->Start(&HttpWebComponent::Invoke, this, httpConfig, request);
    }
    void HttpWebComponent::Invoke(const HttpMethodConfig* config, const std::shared_ptr<Http::Request>& request)
    {
		this->mSumCount++;
        this->mWaitCount++;
		const std::string & address = request->From();
        HttpService* httpService = this->GetComponent<HttpService>(config->Service);
        if (httpService == nullptr || !httpService->IsStartService())
        {           
            this->Send(address, HttpStatus::NOT_FOUND);
            LOG_ERROR("[" << address << "] <<" << request->Url() << ">>" << HttpStatusToString(HttpStatus::NOT_FOUND));         
        }
        else
        {
            const std::string& method = config->Method;
            std::shared_ptr<Http::DataResponse> response(new Http::DataResponse());
            int code = httpService->Invoke(method, request, response);
            if (code != XCode::Successful)
            {
#ifdef __DEBUG__
                LOG_ERROR("[" << config->Type << "] " << config->Path 
                    << " : " << CodeConfig::Inst()->GetDesc(code));
#endif
            }
            this->Send(address, response);
        }            
        this->mWaitCount--;
    }

    void HttpWebComponent::OnRecord(Json::Writer&document)
    {
        document.Add("sum").Add(this->mSumCount);
        document.Add("wait").Add(this->mWaitCount);
    }

    bool HttpWebComponent::OnDelClient(const std::string& address)
    {
        auto iter = this->mTasks.find(address);
        if (iter != this->mTasks.end())
        {           
            this->mTasks.erase(iter);
        }
        return true;
    }
	void HttpWebComponent::OnDestroy()
	{
		this->StopListen();
	}

}