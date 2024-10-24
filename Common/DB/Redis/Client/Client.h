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
	class Client : public tcp::Client
	{
	public:
		typedef acs::IRpc<Request, Response> Component;
		Client(tcp::Socket * socket, const Config & config, Component * component);
	public:
		bool Start();
		void StartReceive();
		void Send(std::unique_ptr<Request> command);
		std::unique_ptr<Response> Sync(std::unique_ptr<Request> command);
	protected:
		void OnConnect(bool result, int count) final;
		std::unique_ptr<Response> ReadResponse(std::unique_ptr<Request> command);
	private:
		void OnResponse();
		void OnSendMessage() final;
		void OnReceiveOnce(int len);
		bool InitRedisClient(const std::string& pwd);
		void OnReadError(const Asio::Code &code) final;
		void OnSendMessage(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &readStream, size_t size) final;
		void OnReceiveMessage(std::istream &readStream, size_t size) final;
	private:
		std::string mAddress;
		Component * mComponent;
		const redis::Config mConfig;
		std::unique_ptr<redis::Request> mRequest;
		std::unique_ptr<redis::Response> mResponse;
	};
}


#endif //APP_CLIENT_H
