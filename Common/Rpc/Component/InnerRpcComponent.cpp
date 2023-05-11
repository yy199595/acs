//
// Created by MyPC on 2023/4/14.
//

#include "InnerRpcComponent.h"
#include"XCode/XCode.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/LocationComponent.h"
#include<google/protobuf/util/json_util.h>
namespace Tendo
{
	InnerRpcComponent::InnerRpcComponent()
		: mRpc("rpc")
	{
		this->mTcpComponent = nullptr;
	}

	bool InnerRpcComponent::LateAwake()
	{
		this->mNodeComponent = this->GetComponent<LocationComponent>();
		this->mTcpComponent = this->GetComponent<InnerNetComponent>();
		return true;
	}

	int InnerRpcComponent::Send(int id, const std::string& func, int proto, long long userId, const Message* message)
	{
		std::string address;
		if(!this->mNodeComponent->GetServerAddress(id, this->mRpc, address))
		{
			return XCode::NetWorkError;
		}
		LOG_ERROR_RETURN_CODE(this->mTcpComponent, XCode::NetWorkError);
		std::shared_ptr<Msg::Packet> request =
				this->MakeTcpRequest(userId, func, proto, message);
		LOG_ERROR_RETURN_CODE(request != nullptr, XCode::MakeTcpRequestFailure);
		return this->mTcpComponent->Send(address, request) ? XCode::Successful : XCode::SendMessageFail;
	}

	int InnerRpcComponent::Send(long long int userId, const std::string & server,
			const string& func, int proto, const google::protobuf::Message* message)
	{
		int targetId = 0;
		ClientUnit * clientUnit = this->mNodeComponent->GetClientById(userId);
		if(clientUnit == nullptr)
		{
			return XCode::NotFindUser;
		}
		clientUnit->Get(server, targetId);
		return this->Send(targetId, func, proto, userId, message);
	}

	std::shared_ptr<Msg::Packet> InnerRpcComponent::Call(long long int userId,
			const string& server, const string& func, int proto, const google::protobuf::Message* message)
	{
		int targetId = 0;
		ClientUnit * clientUnit = this->mNodeComponent->GetClientById(userId);
		if(clientUnit == nullptr)
		{
			return nullptr;
		}
		clientUnit->Get(server, targetId);
		return this->Call(targetId, func, proto, userId, message);
	}

	std::shared_ptr<Msg::Packet> InnerRpcComponent::Call(int id,
			const string& func, int proto, long long int userId, const google::protobuf::Message* message)
	{
		std::string address;
		LOG_CHECK_RET_NULL(this->mTcpComponent);
		std::shared_ptr<Msg::Packet> request =
				this->MakeTcpRequest(userId, func, proto, message);
		LOG_CHECK_RET_NULL(request != nullptr);
		if(!this->mNodeComponent->GetServerAddress(id, this->mRpc, address))
		{
			return nullptr;
		}
		return this->mTcpComponent->Call(address, request);
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

	int InnerRpcComponent::Send(const string& func, const string& server,
			int proto, const google::protobuf::Message* message)
	{
		return 0;
	}

	bool InnerRpcComponent::Send(const string& address, int code, const std::shared_ptr<Msg::Packet>& message)
	{
		LOG_CHECK_RET_FALSE(this->mTcpComponent);
		return this->mTcpComponent->Send(address, code, message);
	}

}