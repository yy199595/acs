#include"HttpDebugComponent.h"
#include"Http/HttpRequest.h"
#include"Http/HttpResponse.h"
#include"String/StringHelper.h"

#include"Config/ClusterConfig.h"
#include"Config/CodeConfig.h"
#include"Json/JsonWriter.h"
#include"Component/NodeMgrComponent.h"
#include"Component/InnerNetMessageComponent.h"
namespace Sentry
{
	bool HttpDebugComponent::LateAwake()
	{
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		this->mInnerComponent = this->GetComponent<InnerNetMessageComponent>();
		return this->StartListen("debug");
	}
	bool HttpDebugComponent::OnDelClient(const std::string& address)
	{
		return true;
	}

	void HttpDebugComponent::OnRequest(const std::string& address, std::shared_ptr<Http::Request> request)
	{
		std::vector<std::string> splitPath;
		const std::string& path = request->Path();
		if (request->Method() != "POST")
		{
			this->Send(address, "please use post call");
		}
		else if (Helper::Str::Split(path, "/", splitPath) != 2)
		{
			this->Send(address, "request path error");
		}
		else
		{
			const std::string& service = splitPath[0];
			const std::string& method = splitPath[1];
			std::string fullName = fmt::format("{0}.{1}", service, method);
			const RpcMethodConfig* config = RpcConfig::Inst()->GetMethodConfig(fullName);
			if (config == nullptr)
			{
				this->Send(address, "call rpc method not find");
				return;
			}
			long long value = 0;
			std::shared_ptr<Rpc::Packet> message(new Rpc::Packet());
			{
				Rpc::Head& head = message->GetHead();
				if (request->Header().Get("id", value))
				{
					head.Add("id", value);
				}
				head.Add("http", address);
				head.Add("func", fullName);
				message->SetProto(Tcp::Porto::Json);
				message->SetType(Tcp::Type::Request);
				std::shared_ptr<Http::PostRequest> postRequest =
					std::dynamic_pointer_cast<Http::PostRequest>(request);
				if (postRequest != nullptr)
				{
					message->Append(postRequest->Content());
				}
			}
			std::string targetAddress;
			if (!this->GetAddress(service, value, targetAddress))
			{
				this->Send(address, "get target address error");
				return;
			}
			TaskComponent* taskComponent = this->mApp->GetTaskComponent();
			taskComponent->Start(&HttpDebugComponent::Invoke, this, targetAddress, message);
		}
	}

	void HttpDebugComponent::Invoke(const std::string& address, std::shared_ptr<Rpc::Packet>& message)
	{
		std::shared_ptr<Rpc::Packet> response =
			this->mInnerComponent->Call(address, message);
		std::shared_ptr<Json::Writer> document = std::make_shared<Json::Writer>();
		if (response == nullptr)
		{
			document->Add("error").Add("unknown error");
		}
		else
		{
			std::string error;
			int code = response->GetCode();
			if (code == XCode::Successful)
			{
				std::string fullName;
				message->GetHead().Get("func", fullName);
				const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
				if (!methodConfig->Response.empty())
				{
					std::unique_ptr<rapidjson::Document> json
						= std::make_unique<rapidjson::Document>();
					const std::string& str = response->GetBody();
					if (json->Parse(str.c_str(), str.size()).HasParseError())
					{
						throw std::logic_error("failed to parse the returned data");
					}
					document->Add("data").Add(*json);
				}
			}
			else if (response->GetHead().Get("error", error))
			{
				document->Add("error").Add(error);
			}
			document->Add("code").Add(CodeConfig::Inst()->GetDesc(code));
		}
		std::string targetAddress;
		message->GetHead().Get("http", targetAddress);
		this->Send(targetAddress, document->JsonString());
	}

	bool HttpDebugComponent::GetAddress(const std::string& service, long long id, std::string& address)
	{		
		std::string server;
		ClusterConfig::Inst()->GetServerName(service, server);
		if (id == 0)
		{
			return this->mNodeComponent->GetServer(server, address);
		}
		return this->mNodeComponent->GetServer(server, id, address);		
	}
}