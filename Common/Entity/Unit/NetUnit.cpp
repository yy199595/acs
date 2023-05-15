//
// Created by leyi on 2023/5/15.
//

#include "NetUnit.h"
#include "App.h"
#include "XCode/XCode.h"
#include "Rpc/Client/Message.h"
#include "Rpc/Component/InnerNetComponent.h"
#include "Rpc/Component/LocationComponent.h"
namespace Tendo
{
	bool NetUnit::LateAwake()
	{
		App * app = App::Inst();
		this->mInnerComponent = app->GetComponent<InnerNetComponent>();
		this->mLocationComponent = app->GetComponent<LocationComponent>();
		LOG_CHECK_RET_FALSE(this->mInnerComponent && this->mLocationComponent);
		return true;
	}


	int NetUnit::NewRequest(const std::string& func, const pb::Message* request, std::shared_ptr<Msg::Packet>& message)
	{
		const RpcMethodConfig * methodConfig = RpcConfig::Inst()->GetMethodConfig(func);
		if(methodConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		message = std::make_shared<Msg::Packet>();
		{
			message->SetType(Msg::Type::Request);
			message->SetProto(Msg::Porto::Protobuf);
			if(!message->WriteMessage(request))
			{
				return XCode::SerializationFailure;
			}
			if(this->GetUnitId() > 0)
			{
				message->GetHead().Add("id", this->GetUnitId());
			}
			message->GetHead().Add("func", func);
		}
		return XCode::Successful;
	}
}
