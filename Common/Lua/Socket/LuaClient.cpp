//
// Created by 64658 on 2025/1/14.
//

#include "LuaClient.h"

namespace lua
{
	Client::Client(Asio::Context & main, tcp::Socket * sock)
		: tcp::Client(sock, 1024 * 1024), mMainContext(main)
	{

	}

	bool Client::Start()
	{
		asio::error_code code;
		return this->ConnectSync(code);
	}

	void Client::Close()
	{
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this()]
		{
			this->StopTimer();
			this->mSocket->Close();
		});
	}

	void Client::OnConnect(bool result, int count)
	{
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(this->mMainContext, [self, result, task = this->mTask.release()]
		{
			task->SetResult(result);
			delete task;
		});
	}

	int Client::Send(const std::string& message, std::unique_ptr<acs::LuaWaitTaskSource> task)
	{
		this->mTask = std::move(task);
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		this->mMessage = std::make_unique<tcp::TextProto>(message);
		asio::post([self, this] { this->Write(*this->mMessage); });
		return this->mTask->Await();
	}

	int Client::Read(std::unique_ptr<acs::LuaWaitTaskSource> task)
	{
		this->mReadTask = std::move(task);
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(context, [self, this] { this->ReadAll(); });
		return this->mReadTask->Await();
	}

	int Client::ReadOneLine(std::unique_ptr<acs::LuaWaitTaskSource> task)
	{
		this->mReadTask = std::move(task);
		Asio::Context & context = this->mSocket->GetContext();
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(context, [self, this] { this->ReadLine(); });
		return this->mReadTask->Await();
	}

	void Client::OnSendMessage(size_t size)
	{
		this->mTask->SetResults(size, "ok");
	}

	void Client::OnSendMessage(const Asio::Code& code)
	{
		if(this->mTask != nullptr)
		{
			this->mTask->SetResults(0, code.message());
			this->mTask.reset();
			this->mSocket->Close();
		}
	}

	void Client::OnReceiveLine(std::istream& readStream, size_t size)
	{
		size_t count = 0;
		std::string message;
		char buffer[128] = { 0 };
		do
		{
			count = readStream.readsome(buffer, sizeof(buffer));
			if (count > 0)
			{
				message.append(buffer, count);
			}
		}
		while(count > 0);
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(this->mMainContext, [self, message, task = this->mReadTask.release()]
		{
			task->SetResults(message.size(), message);
			delete task;
		});
	}

	void Client::OnReceiveMessage(std::istream& readStream, size_t size, const asio::error_code& code)
	{
		size_t count = 0;
		std::string message;
		char buffer[128] = { 0 };
		do
		{
			count = readStream.readsome(buffer, sizeof(buffer));
			if (count > 0)
			{
				message.append(buffer, count);
			}
		}
		while(count > 0);
		std::shared_ptr<tcp::Client> self = this->shared_from_this();
		asio::post(this->mMainContext, [self, message, task = this->mReadTask.release()]
		{
			task->SetResults(self, message);
			delete task;
		});
	}

	void Client::OnReadError(const Asio::Code& code)
	{
		if(this->mReadTask != nullptr)
		{
			std::shared_ptr<tcp::Client> self = this->shared_from_this();
			asio::post(this->mMainContext, [self, code, task = this->mReadTask.release()]
			{
				task->SetResults(0, code.message());
				delete task;
			});
			this->mSocket->Close();
		}
	}

}