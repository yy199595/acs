//
// Created by mac on 2022/5/18.
//

#ifndef APP_MONGOCLIENT_H
#define APP_MONGOCLIENT_H
#include"MongoProto.h"
#include"Mongo/Config/MongoConfig.h"
#include"Network/Tcp/Client.h"
#include"Network/Tcp/Socket.h"
#include"Entity/Component/IComponent.h"

namespace mongo
{
	class Client final : public tcp::Client
	{
	 public:
		typedef acs::IRpc<Request, Response> Component;
		Client(int id, mongo::Config config, Asio::Context & io);
		Client(int id, Component * component, mongo::Config config, Asio::Context & io);
	public:
        void Stop();
		bool Start(tcp::Socket * socket);
		void Send(std::unique_ptr<Request>& request);
		const std::string & GetVersion() const { return this->mInfo.version; }
		bool SyncSend(const std::unique_ptr<Request>& request, mongo::Response & response);
		std::unique_ptr<mongo::Response> SyncMongoCommand(std::unique_ptr<Request>& request);
	private:
		bool Auth(bool connect);
		void OnResponse(int code);
		std::unique_ptr<Response> ReadResponse(const std::string & cmd);
		void OnReceiveMessage(std::istream & is, size_t, const Asio::Code &) final;
		std::unique_ptr<Response> SyncSendMongoCommand(const std::unique_ptr<Request>& request);
		void OnResponse(int code, std::unique_ptr<Request>& request, std::unique_ptr<Response>& response);
	private:
		bool AuthBySha1(const std::string & user, const std::string & db, const std::string & pwd);
#ifdef __ENABLE_OPEN_SSL__
		bool AuthBySha256(const std::string & user, const std::string & db, const std::string & pwd);
#endif
	private:
		void OnSendMessage(size_t size) final;
		void OnReadError(const Asio::Code &code) final;
		void OnConnect(const Asio::Code &, int count) final;
		void OnSendMessage(const asio::error_code &code) final;
	private:
		int mClientId;
		mongo::Info mInfo;
		Component * mComponent;
		const mongo::Config mConfig;
		asio::streambuf streamBuffer;
		Asio::Context & mMainContext;
		std::unique_ptr<mongo::Request> mRequest;
		std::unique_ptr<mongo::Response> mResponse;
	};
}


#endif //APP_MONGOCLIENT_H
