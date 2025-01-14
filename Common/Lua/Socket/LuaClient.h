//
// Created by 64658 on 2025/1/14.
//

#ifndef APP_LUACLIENT_H
#define APP_LUACLIENT_H

#include "Network/Tcp/Client.h"
#include "Lua/Engine/LuaInclude.h"
#include "Async/Lua/LuaWaitTaskSource.h"
namespace lua
{
	class Client : public tcp::Client
	{
	public:
		Client(Asio::Context & main, tcp::Socket * sock);
		~Client() final { printf("----------------\n");}
	public:
		bool Start();
		void Close();
		int Read(std::unique_ptr<acs::LuaWaitTaskSource> task);
		int ReadOneLine(std::unique_ptr<acs::LuaWaitTaskSource> task);
		int Send(const std::string & message, std::unique_ptr<acs::LuaWaitTaskSource> task);
	private:
		void OnSendMessage(size_t size) final;
		void OnReadError(const Asio::Code &code) final;
		void OnConnect(bool result, int count) final;
		void OnSendMessage(const Asio::Code &code) final;
		void OnReceiveLine(std::istream &readStream, size_t size) final;
		void OnReceiveMessage(std::istream &readStream, size_t size, const asio::error_code &code) final;
	private:
		Asio::Context & mMainContext;
		std::unique_ptr<tcp::TextProto> mMessage;
		std::unique_ptr<acs::LuaWaitTaskSource> mTask;
		std::unique_ptr<acs::LuaWaitTaskSource> mReadTask;
	};
}


#endif //APP_LUACLIENT_H
