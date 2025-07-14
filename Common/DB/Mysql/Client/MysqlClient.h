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
		Client(int id, Component * component, mysql::Config  config, Asio::Context & io);
	public:
		void StartReceive();
		bool InvokeCompileSql();
		int Start(tcp::Socket* socket);
		void Send(std::unique_ptr<Request> request);
		const mysql::HandshakeResponse & GetHandshake() const { return this->mHandshake; }
	private:
		void OnSendMessage(size_t size) final;
		void OnReadError(const Asio::Code &code) final;
		void OnSendMessage(const Asio::Code &code) final;
		void OnConnect(const Asio::Code &code, int count) final;
		void OnReceiveLine(std::istream &readStream, size_t size) final;
	private:
		void Send(mysql::Request & request);
		std::unique_ptr<mysql::Response> ReadAllPacket();
		bool SyncReadOnePacket(mysql::Response & response);
		bool SyncRun(const std::string & sql, mysql::Response & response);
	private:
		bool InitDataBase();
		bool OnCompileSql();
		int Auth(bool connect);
		//int OnMoreAuth(unsigned char status);
		int Login(const mysql::LoginRequest & request);
		void OnTextMessage(json::w::Document & document);
		bool SendPacket(unsigned char * message, size_t size, unsigned char id);
		void OnReceiveMessage(std::istream &readStream, size_t size, const asio::error_code &code) final;
	private:
		int mClientID;
		std::string mBuffer;
		unsigned char mIndex;
		mysql::Config mConfig;
		Asio::Context & mMain;
		Component * mComponent;
		mysql::Response mResponse;
		std::unique_ptr<Request> mMessage;
		mysql::HandshakeResponse mHandshake;
		std::vector<mysql::FieldInfo> mFields;
	};
}


#endif //APP_MYSQL_CLIENT_H
