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
		int Send(int id, const std::string & func, int proto, long long userId, const Message * message = nullptr);
		int Send(const std::string & func, const std::string & server, int proto, const Message * message);
		int Send(long long userId, const std::string & server, const std::string & func, int proto, const Message * message = nullptr);
	public:
		std::shared_ptr<Msg::Packet> Call(int id, const std::string & func, int proto, long long userId, const google::protobuf::Message * message = nullptr);
		std::shared_ptr<Msg::Packet> Call(long long userId, const std::string & server, const std::string & func, int proto, const Message * message = nullptr);
	public:
		std::shared_ptr<Msg::Packet> MakeTcpRequest(long long userId, const std::string & func, int protoc, const google::protobuf::Message * message);
	public:
		bool Send(const std::string & address, int code, const std::shared_ptr<Msg::Packet> & message);
	private:
		bool LateAwake() final;
	private:
		const std::string mRpc;
		class InnerNetComponent * mTcpComponent;
		class LocationComponent * mNodeComponent;
	};
}


#endif //APP_INNERRPCCOMPONENT_H
