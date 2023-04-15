//
// Created by MyPC on 2023/4/14.
//

#ifndef APP_INNERRPCCOMPONENT_H
#define APP_INNERRPCCOMPONENT_H
#include"Entity/Component/Component.h"
#include "Rpc/Client/Message.h"
#include "Http/Common/HttpRequest.h"
#include<google/protobuf/message.h>

namespace Tendo
{
	class InnerRpcComponent : public Component
	{
	public:
		InnerRpcComponent();
	public:
		int Send(const std::string & func, const std::string & server, int proto, const google::protobuf::Message * message);
		int Send(long long userId, const std::string & server, const std::string & func, int proto, const google::protobuf::Message * message = nullptr);
		int Send(const std::string & address, const std::string & func, int proto, long long userId = 0, const google::protobuf::Message * message = nullptr);
	public:
		std::shared_ptr<Rpc::Packet> Call(long long userId, const std::string & server, const std::string & func, int proto, const google::protobuf::Message * message = nullptr);
		std::shared_ptr<Rpc::Packet> Call(const std::string & address, const std::string & func, int proto, long long userId = 0, const google::protobuf::Message * message = nullptr);
	public:
		bool FormatUrl(const std::string & address, const std::string & func, std::string & url);
		std::shared_ptr<Rpc::Packet> MakeTcpRequest(long long userId, const std::string & func, int protoc, const google::protobuf::Message * message);
		std::shared_ptr<Http::PostRequest> MakeHttpRequest(long long userId, const std::string & func, int protoc, const google::protobuf::Message * message);
	public:
		bool Send(const std::string & address, int code, const std::shared_ptr<Rpc::Packet> & message);
	private:
		bool LateAwake() final;
	private:
		const std::string mTcp;
		const std::string mHttp;
		class HttpComponent * mHttpComponent;
		class HttpWebComponent * mWebComponent;
		class InnerNetComponent * mTcpComponent;
		class NodeMgrComponent * mNodeComponent;
	};
}


#endif //APP_INNERRPCCOMPONENT_H
