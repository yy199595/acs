//
// Created by mac on 2022/5/18.
//

#ifndef APP_MONGOCLIENT_H
#define APP_MONGOCLIENT_H
#include"MongoProto.h"
#include"Mongo/Config/MongoConfig.h"
#include"Network/Tcp/TcpClient.h"
#include"Network/Tcp/Socket.h"
#include"Entity/Component/IComponent.h"

namespace mongo
{
	class Client : public tcp::TcpClient
	{
	 public:
		typedef joke::IRpc<Request, Response> Component;
		Client(tcp::Socket * socket, const MongoConfig & config);
		Client(tcp::Socket * socket, Component * component, const MongoConfig& config);
	public:
        void Stop();
		bool Start(bool async = true);
		void SendMongoCommand(std::unique_ptr<Request> request);
		std::unique_ptr<mongo::Response> SyncMongoCommand(std::unique_ptr<Request> request);
	private:
		bool StartAuthBySha1();
		void OnResponse(int code);
		std::unique_ptr<Response> ReadResponse();
		void OnTimeout(tcp::TimeoutFlag flag) final;
		void OnReadError(const Asio::Code &code) final;
		void OnReceiveMessage(std::istream & is, size_t) final;
		void OnResponse(int code, Request * request, Response * response);
        bool Auth(const std::string & user, const std::string & db, const std::string & pwd);
		std::unique_ptr<Response> SyncSendMongoCommand(std::unique_ptr<Request> request);
	private:
		void OnSendMessage() final;
		void OnConnect(bool result, int count) final;
		void OnSendMessage(const asio::error_code &code) final;
	private:
		Component * mComponent;
		const MongoConfig mConfig;
		mongo::Request * mRequest;
		mongo::Response * mResponse;
		asio::streambuf streamBuffer;
	};
}


#endif //APP_MONGOCLIENT_H
