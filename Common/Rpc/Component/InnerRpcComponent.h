//
// Created by MyPC on 2023/4/14.
//

#ifndef APP_INNERRPCCOMPONENT_H
#define APP_INNERRPCCOMPONENT_H
#include"Entity/Component/Component.h"
#include "Rpc/Client/Message.h"
#include "Http/Common/HttpRequest.h"
#include<google/protobuf/message.h>
using namespace google::protobuf;
namespace Tendo
{
	class InnerRpcComponent : public Component
	{
	public:
		InnerRpcComponent();
	public:
		bool Send(const std::string & address, int code, const std::shared_ptr<Msg::Packet> & message);
		int Send(const std::string & func, const std::string & server, int proto, const Message * message);
		int Send(const std::string & addr, const std::string & func, int proto, long long userId, const Message * message = nullptr);
		std::shared_ptr<Msg::Packet> MakeRequest(long long userId, const std::string & func, int protoc, const google::protobuf::Message * message);
		std::shared_ptr<Msg::Packet> Call(const std::string & addr, const std::string & func, int proto, long long userId, const google::protobuf::Message * message = nullptr);
	private:
		bool LateAwake() final;
	private:
		const std::string mRpc;
		class InnerNetComponent * mTcpComponent;
		class LocationComponent * mNodeComponent;
	};
}


#endif //APP_INNERRPCCOMPONENT_H
