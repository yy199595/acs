//
// Created by yy on 2025/2/10.
//

#ifndef APP_MYSQL_CLIENT_H
#define APP_MYSQL_CLIENT_H

#include "Network/Tcp/Client.h"
#include "Mysql/Common/MysqlConfig.h"
#include "Mysql/Common/MysqlProto.h"
#include "Entity/Component/IComponent.h"

namespace mysql
{
	class Client : public tcp::Client
	{
	public:
		typedef acs::IRpc<Request, Response> Component;
		Client(int id, tcp::Socket * socket, Component * component, mysql::Config  config, Asio::Context & io);
	public:
		bool Connect();
		void Send(std::unique_ptr<Request> request);
	private:
		void OnSendMessage(size_t size) final;
		void OnSendMessage(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &readStream, size_t size) final;
	private:
		bool SyncReadResponse(std::string & response);
		bool SyncReadResponse(mysql::Response & response);
		bool SyncRun(const std::string & sql, mysql::Response & response);
	private:
		bool SwitchDataBase();
		bool Auth(mysql::Response & response);
		void OnReadError(const Asio::Code &code) final;
		void OnReceiveMessage(std::istream &readStream, size_t size, const asio::error_code &code) final;
	private:
		int mClientID;
		unsigned char mIndex;
		Asio::Context & mMain;
		mysql::Config mConfig;
		Component * mComponent;
		mysql::Response mResponse;
		mysql::HandshakeResponse mHandshake;
		std::vector<mysql::FieldInfo> mFields;
		std::queue<std::unique_ptr<Request>> mMessages;
	};
}


#endif //APP_MYSQL_CLIENT_H
