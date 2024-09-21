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
		typedef acs::IRpc<Request, Response> Component;
		Client(tcp::Socket * socket, const MongoConfig & config);
		Client(tcp::Socket * socket, Component * component, const MongoConfig& config);
	public:
        void Stop();
		bool Start(bool async = true);
		void SendMongoCommand(std::unique_ptr<Request> request);
		bool SyncSend(std::unique_ptr<Request> request, mongo::Response & response);
		std::unique_ptr<mongo::Response> SyncMongoCommand(std::unique_ptr<Request> request);
	private:
		bool StartAuthBySha1();
		void OnResponse(int code);
		std::unique_ptr<Response> ReadResponse();
		void OnTimeout(tcp::TimeoutFlag flag) final;
		void OnReadError(const Asio::Code &code) final;
		void OnReceiveMessage(std::istream & is, size_t) final;
		std::unique_ptr<Response> SyncSendMongoCommand(std::unique_ptr<Request> request);
        bool Auth(const std::string & user, const std::string & db, const std::string & pwd);
		void OnResponse(int code, std::unique_ptr<Request> request, std::unique_ptr<Response> response);
	private:
		void OnSendMessage() final;
		void OnConnect(bool result, int count) final;
		void OnSendMessage(const asio::error_code &code) final;
	private:
		Component * mComponent;
		const MongoConfig mConfig;
		asio::streambuf streamBuffer;
		std::unique_ptr<mongo::Request> mRequest;
		std::unique_ptr<mongo::Response> mResponse;
	};
}


#endif //APP_MONGOCLIENT_H
