#include"HttpDebugComponent.h"
#include"Http/Common/HttpRequest.h"
#include"Http/Common/HttpResponse.h"
#include"Util/String/StringHelper.h"
#include"Entity/App/App.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Server/Config/CodeConfig.h"
#include"Util//Json/JsonWriter.h"
#include"Rpc/Component/NodeMgrComponent.h"
#include"Rpc/Component/InnerNetComponent.h"

namespace Tendo
{
	bool HttpDebugComponent::LateAwake()
	{
		this->mNodeComponent = this->GetComponent<NodeMgrComponent>();
		this->mInnerComponent = this->GetComponent<InnerNetComponent>();
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
			std::string target;
			if (!this->GetAddress(service, value, target))
			{
				this->Send(address, "get target address error");
				return;
			}
			AsyncMgrComponent* taskComponent = this->mApp->GetTaskComponent();
			taskComponent->Start(&HttpDebugComponent::Call, this, target, message);
		}
	}

	void HttpDebugComponent::Call(const std::string& address, std::shared_ptr<Rpc::Packet>& message)
	{
		std::shared_ptr<Rpc::Packet> response =
			this->mInnerComponent->Call(address, message);
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
				const RpcMethodConfig* methodConfig = RpcConfig::Inst()->GetMethodConfig(fullName);
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
		ClusterConfig::Inst()->GetServerName(service, server);
		if (id == 0)
		{
			return this->mNodeComponent->GetServer(server, address);
		}
		return this->mNodeComponent->GetServer(server, id, address);		
	}
}