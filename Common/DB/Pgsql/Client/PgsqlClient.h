//
// Created by 64658 on 2025/2/18.
//

#ifndef APP_PGSQL_CLIENT_H
#define APP_PGSQL_CLIENT_H
#include "Network/Tcp/Client.h"
#include "Pgsql/Common/PgsqlCommon.h"
#include "Entity/Component/IComponent.h"

namespace pgsql
{
	typedef acs::IRpc<Request, Response> Component;
	class Client : public tcp::Client
	{
	public:
		Client(int id, Component * component, pgsql::Config & config, Asio::Context & main);
	public:
		int Start(tcp::Socket * socket);
		void Send(std::unique_ptr<pgsql::Request> request);
	private:
		int Auth(bool connect);
#ifdef __ENABLE_OPEN_SSL__
		int AuthBySha256();
#endif
		void OnSendMessage(size_t size) final;
		void OnReadError(const Asio::Code &code) final;
		void OnConnect(const Asio::Code &code, int count) final;
		void OnReceiveMessage(std::istream &readStream, size_t size, const asio::error_code &code) final;
	private:
		bool CreateDatabase();
		bool ReadResponse(pgsql::Result & response);
		bool ReadResponse(char& type, std::string & response);
		bool ReadResponse(const std::string & sql, pgsql::Result & response);
		int Auth(unsigned int type, const std::string & message);
	private:
		int mClientId;
		Asio::Context & mMain;
		pgsql::Config mConfig;
		Component * mComponent;
		pgsql::Result mResponse;
		std::unique_ptr<pgsql::Request> mRequest;
	};
}


#endif //APP_PGSQL_CLIENT_H
