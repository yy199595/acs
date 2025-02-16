//
// Created by leyi on 2023/7/27.
//

#ifndef APP_CLIENT_H
#define APP_CLIENT_H

#include"RedisDefine.h"
#include"Network/Tcp/Client.h"
#include"Redis/Config/RedisConfig.h"
#include"Entity/Component/IComponent.h"

namespace redis
{
	class Client final : public tcp::Client
	{
	public:
		typedef acs::IRpc<Request, Response> Component;
		Client(tcp::Socket * socket, const Config & config, Component * component, Asio::Context &);
	public:
		bool Start();
		void StartReceive();
		void Send(std::unique_ptr<Request> command);
		std::unique_ptr<Response> Sync(std::unique_ptr<Request> command);
	protected:
		void OnConnect(const Asio::Code & code, int count) final;
		std::unique_ptr<Response> ReadResponse(const std::unique_ptr<Request>& command);
	private:
		void OnResponse();
		void OnReceiveOnce(int len);
		void OnSendMessage(size_t size) final;
		bool InitRedisClient(const std::string& pwd);
		void OnReadError(const Asio::Code &code) final;
		void OnSendMessage(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &readStream, size_t size) final;
		void OnReceiveMessage(std::istream &readStream, size_t size, const Asio::Code &) final;
	private:
		std::string mAddress;
		Component * mComponent;
		const redis::Config mConfig;
		Asio::Context & mMainContext;
		std::unique_ptr<redis::Request> mRequest;
		std::unique_ptr<redis::Response> mResponse;
	};
}


#endif //APP_CLIENT_H
