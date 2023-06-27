//
// Created by zmhy0073 on 2022/6/21.
//

#include"HttpWebComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Config/CodeConfig.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Http/Client/HttpHandlerClient.h"
#include"Http/Service/HttpService.h"
#include"Util/File/DirectoryHelper.h"
#include"Util/File/FileHelper.h"
#include"Server/Component/ThreadComponent.h"
#include"Rpc/Component/DispatchComponent.h"
namespace Tendo
{
    HttpWebComponent::HttpWebComponent()
        : mSumCount(0), mWaitCount(0)
    {
		this->mTaskComponent = nullptr;
		this->mDispatchComponent = nullptr;
        this->mTypeContent["js"] = Http::ContentName::JS;
        this->mTypeContent["css"] = Http::ContentName::CSS;
        this->mTypeContent["json"] = Http::ContentName::JSON;
        this->mTypeContent["html"] = Http::ContentName::HTML;
        this->mTypeContent["txt"] = Http::ContentName::TEXT;
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
		if(this->mApp->Config()->GetPath("html", dir))
		{
			this->AddStaticDir(dir);
		}
        this->mTaskComponent = this->mApp->GetCoroutine();
        ServerConfig::Inst()->GetPath("home", this->mHomePath);
		this->mDispatchComponent = this->GetComponent<DispatchComponent>();
		return this->StartListen("http");
    }

	void HttpWebComponent::AddStaticDir(const std::string& dir)
	{
		std::vector<std::string> files;
		Helper::Directory::GetFilePaths(dir, files);
		for(const std::string & fullPath : files)
		{
            std::string type;
			Http::StaticSource source;
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
        this->mSumCount++;
		std::shared_ptr<Http::IResponse> response;
		const std::string & address = request->From();
		do
		{
			const std::string & path = request->Path();
			if(path == "/")
			{
				response = std::make_shared<Http::FileResponse>(this->mHomePath);

				response->SetCode(HttpStatus::OK);
				response->Header().Add(Http::HeadName::ContentType, Http::ContentName::HTML);
				response->Header().Add(Http::HeadName::ContentLength, (int)response->ContentSize());
				break;
			}
			const HttpMethodConfig *httpConfig = HttpConfig::Inst()->GetMethodConfig(path);
			if(httpConfig != nullptr)
			{
				if (!httpConfig->Type.empty() && httpConfig->Type != request->Method())
				{
					response = std::make_shared<Http::DataResponse>();
					response->SetCode(HttpStatus::METHOD_NOT_ALLOWED);
					LOG_ERROR("[" << address << "] <<" << path << ">>"
						<< HttpStatusToString(HttpStatus::METHOD_NOT_ALLOWED));
					break;
				}
				if (!httpConfig->IsAsync)
				{
					this->Invoke(httpConfig, request);
					return;
				}
				this->mTaskComponent->Start(&HttpWebComponent::Invoke, this, httpConfig, request);
				return;
			}

			auto iter = this->mStaticSourceDir.find(path);
			if (iter != this->mStaticSourceDir.end())
			{
				Http::StaticSource& source = iter->second;
				response = std::make_shared<Http::FileResponse>(source.mPath);
				response->SetCode(HttpStatus::OK);
				response->Header().Add(Http::HeadName::ContentType, source.mType);
				response->Header().Add(Http::HeadName::ContentLength, (int)response->ContentSize());
				break;
			}
			if(this->OnMessage(request))
			{
				return;
			}
			response = std::make_shared<Http::DataResponse>();
			response->SetCode(HttpStatus::METHOD_NOT_ALLOWED);
			LOG_ERROR("[" << address << "] <<" << path
						  << ">>" << HttpStatusToString(HttpStatus::NOT_FOUND));
		}
		while(false);
		this->Send(address, response);
    }

    bool HttpWebComponent::OnMessage(const std::shared_ptr<Http::Request>& request)
    {
		if(this->mDispatchComponent == nullptr)
		{
			return false;
		}
		std::vector<std::string> result;
        const std::string & path = request->Path();
		size_t pos = path.find('/',1);
		if(pos == std::string::npos || pos <= 1)
		{
			return false;
		}
		std::string func(path.c_str() + 1, path.size() - 1);
		func[pos - 1] = '.';
		if(SrvRpcConfig::Inst()->GetMethodConfig(func) == nullptr)
		{
			return false;
		}
		std::shared_ptr<Msg::Packet> message
			= std::make_shared<Msg::Packet>();
		if(request->Header().ContentType() == Http::ContentName::PB)
		{
			message->SetProto(Msg::Porto::Protobuf);
			message->SetContent(request->Content());
		}
		else if(request->Header().ContentType() == Http::ContentName::JSON)
		{
			message->SetProto(Msg::Porto::Json);
			message->SetContent(request->Content());
		}
		{
			message->SetNet(Msg::Net::Http);
			message->SetFrom(request->From());
			message->SetType(Msg::Type::Request);
			message->GetHead().Add("func", func);
		}
		int code = this->mDispatchComponent->OnMessage(message);
		if(code != XCode::Successful)
		{
			this->SendData(request->From(), code, nullptr);
		}
		return true;
	}

    void HttpWebComponent::Invoke(const HttpMethodConfig* config, const std::shared_ptr<Http::Request>& request)
    {
        this->mWaitCount++;
		const std::string & address = request->From();
        HttpService* httpService = this->GetComponent<HttpService>(config->Service);
        if (httpService == nullptr)
        {           
            this->Send(address, HttpStatus::NOT_FOUND);
            LOG_ERROR("[" << address << "] <<" << request->Url() << ">>" << HttpStatusToString(HttpStatus::NOT_FOUND));         
        }
        else
        {
            const std::string& method = config->Method;
            std::shared_ptr<Http::DataResponse> response(new Http::DataResponse());
            int code = httpService->Invoke(method, request, response);
            if (code != XCode::Successful && code != XCode::LuaCoroutineWait)
            {
#ifdef __DEBUG__
                LOG_ERROR("[" << config->Type << "] " << config->Path 
                    << " : error = " << CodeConfig::Inst()->GetDesc(code));
#endif
            }
            this->Send(address, response);
        }
        this->mWaitCount--;
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
		this->ClearClients();
	}

	bool HttpWebComponent::SendData(const string& address, int code, const std::shared_ptr<Msg::Packet>& message)
	{
		std::shared_ptr<Http::DataResponse> response = std::make_shared<Http::DataResponse>();
		response->Header().Add("code", code);
		if (code == XCode::Successful && message != nullptr)
		{
			response->Content(HttpStatus::OK, Http::ContentName::JSON, message->GetBody());
		}
		else
		{
			const std::string & desc = CodeConfig::Inst()->GetDesc(code);
			response->Content(HttpStatus::OK, Http::ContentName::TEXT, desc);
		}
		return this->Send(address, response);
	}

}