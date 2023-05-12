//
// Created by MyPC on 2023/4/14.
//

#include "InnerRpcComponent.h"
#include"XCode/XCode.h"
#include"Http/Component/HttpComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/LocationComponent.h"
#include "Http/Common/HttpRequest.h"
#include "Http/Common/HttpResponse.h"
#include"Http/Component/HttpWebComponent.h"
#include<google/protobuf/util/json_util.h>
namespace Tendo
{
	InnerRpcComponent::InnerRpcComponent()
		: mTcp("tcp"), mHttp("http")
	{
		this->mTcpComponent = nullptr;
		this->mHttpComponent = nullptr;
	}

	bool InnerRpcComponent::LateAwake()
	{
		this->mHttpComponent = this->GetComponent<HttpComponent>();
		this->mWebComponent = this->GetComponent<HttpWebComponent>();
		this->mNodeComponent = this->GetComponent<LocationComponent>();
		this->mTcpComponent = this->GetComponent<InnerNetComponent>();
		return true;
	}

	int InnerRpcComponent::Send(const std::string& address, const std::string& func, int proto,
			long long userId, const google::protobuf::Message* message)
	{
		if(address.find(this->mTcp) == 0)
		{
			LOG_ERROR_RETURN_CODE(this->mTcpComponent, XCode::NetWorkError);
			std::shared_ptr<Msg::Packet> request =
					this->MakeTcpRequest(userId, func, proto, message);
			LOG_ERROR_RETURN_CODE(request != nullptr, XCode::MakeTcpRequestFailure);
			return this->mTcpComponent->Send(address, request) ? XCode::Successful : XCode::SendMessageFail;
		}
		else if(address.find(this->mHttp) == 0)
		{
			LOG_ERROR_RETURN_CODE(this->mHttpComponent, XCode::NetWorkError);

			std::shared_ptr<Http::PostRequest> request =
					this->MakeHttpRequest(userId, func, proto, message);
			LOG_ERROR_RETURN_CODE(request != nullptr, XCode::MakeHttpRequestFailure);

			std::string url;
			LOG_ERROR_RETURN_CODE(this->FormatUrl(address, func, url)
								  && request->SetUrl(url), XCode::ParseHttpUrlFailure);
			return this->mHttpComponent->Send(request) ? XCode::Successful : XCode::SendMessageFail;
		}
		LOG_ERROR("unknown address : [" << address << "]");
		return XCode::UnknownMessageNetType;
	}

	int InnerRpcComponent::Send(long long int userId, const std::string & server,
			const string& func, int proto, const google::protobuf::Message* message)
	{
		std::string  address;
		if(!this->mNodeComponent->GetServer(server, userId, address))
		{
			return XCode::NotFindUser;
		}
		return this->Send(address, func, proto, userId, message);
	}

	std::shared_ptr<Msg::Packet> InnerRpcComponent::Call(long long int userId,
			const string& server, const string& func, int proto, const google::protobuf::Message* message)
	{
		std::string address;
		if(!this->mNodeComponent->GetServer(server, userId, address))
		{
			LOG_ERROR("not find user address user_id:" << userId << " server:" << server);
			return nullptr;
		}
		return this->Call(address, func, proto, userId, message);

	}

	std::shared_ptr<Msg::Packet> InnerRpcComponent::Call(const string& address,
			const string& func, int proto, long long int userId, const google::protobuf::Message* message)
	{
		LOG_WARN("call [" << address << "] func = " << func);
		if(address.find(this->mTcp) == 0)
		{
			LOG_CHECK_RET_NULL(this->mTcpComponent);
			std::shared_ptr<Msg::Packet> request =
					this->MakeTcpRequest(userId, func, proto, message);
			LOG_CHECK_RET_NULL(request != nullptr);
			return this->mTcpComponent->Call(address, request);
		}
		else if(address.find(this->mHttp) == 0)
		{
			LOG_CHECK_RET_NULL(this->mHttpComponent);
			std::shared_ptr<Http::PostRequest> request =
					this->MakeHttpRequest(userId, func, proto, message);
			LOG_CHECK_RET_NULL(request != nullptr);

			std::string url;
			LOG_CHECK_RET_NULL(this->FormatUrl(address, func, url) && request->SetUrl(url));
			std::shared_ptr<Http::DataResponse> response = this->mHttpComponent->Await(request);
			if(response == nullptr || response->Code() != HttpStatus::OK)
			{
				return nullptr;
			}
			std::shared_ptr<Msg::Packet> rpcResponse
					= std::make_shared<Msg::Packet>();
			{
				rpcResponse->SetProto(proto);
				rpcResponse->SetType(Msg::Type::Response);
			}
			int code = XCode::Failure;
			if(response->Header().Get("code", code))
			{
				rpcResponse->GetHead().Add("code", code);
			}
			rpcResponse->SetContent(response->GetContent());
			return rpcResponse;
		}
		LOG_ERROR("unknown address : [" << address << "]");
		return nullptr;
	}

	std::shared_ptr<Msg::Packet> InnerRpcComponent::MakeTcpRequest(long long int userId, const string& func, int proto,
			const google::protobuf::Message* message)
	{
		std::shared_ptr<Msg::Packet> request
			= std::make_shared<Msg::Packet>();
		request->SetProto(proto);
		request->SetType(Msg::Type::Request);
		if (userId != 0)
		{
			request->GetHead().Add("id", userId);
		}
		request->GetHead().Add("func", func);
		return request->WriteMessage(message) ? request : nullptr;
	}

	std::shared_ptr<Http::PostRequest> InnerRpcComponent::MakeHttpRequest(long long int userId, const string& func, int proto,
			const google::protobuf::Message* message)
	{
		std::shared_ptr<Http::PostRequest> request
			= std::make_shared<Http::PostRequest>();
		if(userId != 0)
		{
			request->Header().Add("id", userId);
		}
		if(message != nullptr)
		{
			switch(proto)
			{
				case Msg::Porto::Json:
				{
					std::string data;
					if(!message->SerializeToString(&data))
					{
						return nullptr;
					}
					request->Protobuf(data);
					break;
				}
				case Msg::Porto::Protobuf:
				{
					std::string json;
					if(!util::MessageToJsonString(*message, &json).ok())
					{
						return nullptr;
					}
					request->Json(json);
					break;
				}
				default:
					return nullptr;
			}
		}
		return request;
	}

	bool InnerRpcComponent::FormatUrl(const string& address, const string& func, string& url)
	{
		url.assign(func);
		size_t pos = url.find('.');
		if(pos == std::string::npos)
		{
			return false;
		}
		url[pos] = '/';
		url = fmt::format("{0}/{1}", address, url);
		return true;
	}

	int InnerRpcComponent::Send(const string& func, const string& server,
			int proto, const google::protobuf::Message* message)
	{
		int count = 0;
		const std::string listen("rpc");
		std::vector<std::string> servers;
		this->mNodeComponent->GetServer(server, listen, servers);
		for(const std::string & address : servers)
		{
			if(this->Send(address, func, proto, message) == XCode::Successful)
			{
				count++;
			}
		}
		return count;
	}

	bool InnerRpcComponent::Send(const string& address, int code, const std::shared_ptr<Msg::Packet>& message)
	{
		if(address.find(this->mTcp) == 0)
		{
			LOG_CHECK_RET_FALSE(this->mTcpComponent);
			return this->mTcpComponent->Send(address, code, message);
		}
		else if(address.find(this->mHttp) == 0)
		{
			LOG_CHECK_RET_FALSE(this->mHttpComponent);
			return this->mWebComponent->SendData(address, code, message);
		}
		return false;
	}

}