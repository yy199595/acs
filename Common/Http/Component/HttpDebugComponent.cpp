#include"HttpDebugComponent.h"
#include"XCode/XCode.h"
#include"Http/Common/HttpRequest.h"
#include"Http/Common/HttpResponse.h"
#include"Util/String/StringHelper.h"
#include"Entity/Actor/App.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Util//Json/JsonWriter.h"
#include"Router/Component/RouterComponent.h"

namespace Tendo
{
	bool HttpDebugComponent::LateAwake()
	{
		this->mRouterComponent = this->GetComponent<RouterComponent>();
		return this->StartListen("debug");
	}
	bool HttpDebugComponent::OnDelClient(const std::string& address)
	{
		return true;
	}

	void HttpDebugComponent::OnRequest(std::shared_ptr<Http::Request> request)
	{
		std::vector<std::string> splitPath;
		const std::string& path = request->Path();
		const std::string& address = request->From();
		if (request->Method() != "POST")
		{
			this->Send(address, "please use post call");
		}
		else if (Helper::Str::Split(path, '/', splitPath) != 2)
		{
			this->Send(address, "request path error");
		}
		else
		{
			const std::string& service = splitPath[0];
			const std::string& method = splitPath[1];
			std::string fullName = fmt::format("{0}.{1}", service, method);
			const RpcMethodConfig* config = SrvRpcConfig::Inst()->GetMethodConfig(fullName);
			if (config == nullptr)
			{
				this->Send(address, "call rpc method not find");
				return;
			}
			long long value = 0;
			std::shared_ptr<Msg::Packet> message(new Msg::Packet());
			{
				Msg::Head& head = message->GetHead();
				if (request->Header().Get("id", value))
				{
					head.Add("id", value);
				}
				head.Add("http", address);
				head.Add("func", fullName);
				message->SetProto(Msg::Porto::Json);
				message->SetType(Msg::Type::Request);
				std::shared_ptr<Http::PostRequest> postRequest =
					std::dynamic_pointer_cast<Http::PostRequest>(request);
				if (postRequest != nullptr)
				{
					message->Append(postRequest->Content());
				}
			}
			std::string target;
			if (!this->GetAddress(service, value, target))
			{
				this->Send(address, "get target address error");
				return;
			}
			CoroutineComponent* taskComponent = this->mApp->GetCoroutine();
			taskComponent->Start(&HttpDebugComponent::Call, this, target, message);
		}
	}

	void HttpDebugComponent::Call(const std::string& address, std::shared_ptr<Msg::Packet>& message)
	{
		std::shared_ptr<Msg::Packet> response =
			this->mRouterComponent->Call(address, message);
		Json::Writer document;
		if (response == nullptr)
		{
			document.Add("error").Add("unknown error");
		}
		else
		{
			std::string error;
			int code = response->GetCode();
			if (code == XCode::Successful)
			{
				std::string fullName;
				message->GetHead().Get("func", fullName);
				const RpcMethodConfig* methodConfig = SrvRpcConfig::Inst()->GetMethodConfig(fullName);
				if (!methodConfig->Response.empty())
				{
					rapidjson::Document json;
					const std::string& str = response->GetBody();
					if (json.Parse(str.c_str(), str.size()).HasParseError())
					{
						throw std::logic_error("failed to parse the returned data");
					}
					document.Add("data").Add(json);
				}
			}
			else if (response->GetHead().Get("error", error))
			{
				document.Add("error").Add(error);
			}
			document.Add("code").Add(CodeConfig::Inst()->GetDesc(code));
		}
		std::string httpAddress;
		message->GetHead().Get("http", httpAddress);
		this->Send(httpAddress, document.JsonString());
	}

	bool HttpDebugComponent::GetAddress(const std::string& service, long long id, std::string& address)
	{		
		std::string server;
		const std::string listen("rpc");
		ClusterConfig::Inst()->GetServerName(service, server);
		if (id == 0)
		{

		}
		return false; //TODO
		//return this->mNodeComponent->GetServerAddress(id, server, listen, address);
	}
}