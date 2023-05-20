//
// Created by MyPC on 2023/4/14.
//

#ifndef APP_INNERRPCCOMPONENT_H
#define APP_INNERRPCCOMPONENT_H
#include"Entity/Component/Component.h"
#include"Rpc/Client/Message.h"
#include"Http/Common/HttpRequest.h"
#include"Proto/Include/Message.h"
namespace Tendo
{
	class InnerRpcComponent : public Component
	{
	public:
		InnerRpcComponent();
	public:
		bool Send(const std::string & address, int code, const std::shared_ptr<Msg::Packet> & message);
		int Send(const std::string & func, const std::string & server, int proto, const pb::Message * message);
		int Send(const std::string & addr, const std::string & func, int proto, long long userId, const pb::Message * message = nullptr);
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
