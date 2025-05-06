//
// Created by 64658 on 2025/4/11.
//

#include "LocalNetComponent.h"
#include "Entity/Actor/App.h"
#include "Server/Component/ThreadComponent.h"

namespace acs
{
	LocalNetComponent::LocalNetComponent()
	{
		this->mThread = nullptr;
	}

	bool LocalNetComponent::LateAwake()
	{
		return true;
	}

	bool LocalNetComponent::StopListen()
	{
		asio::error_code code;
		return this->mAcceptor->cancel(code).value() == Asio::OK;
	}

	bool LocalNetComponent::StartListen(const acs::ListenConfig& listen)
	{
		this->mThread = this->GetComponent<ThreadComponent>();
		Asio::Context & asioContext = this->mThread->GetContext();

		try
		{
			asio::local::stream_protocol::endpoint localEndpoint(R"(\\.\pipe\my_new_named_pipe)");
			//LocalEndpoint localEndpoint(path);
			this->mAcceptor = std::make_unique<LocalAcceptor>(asioContext);

			this->mAcceptor->open(asio::local::stream_protocol());
			this->mAcceptor->set_option(asio::socket_base::reuse_address(true));

			this->mAcceptor->bind(localEndpoint);
		}
		catch (const std::system_error & error)
		{
			LOG_ERROR("{}", error.what());
			return false;
		}
		this->mExecutor = asioContext.get_executor();
		asio::post(asioContext, [this]() { this->StartAcceptor(); });

		return true;
	}

	void LocalNetComponent::StartAcceptor()
	{
		Asio::Context& context = this->mThread->GetContext();
		LocalSocket * localSocket = new LocalSocket(context);
		this->mAcceptor->async_accept(*localSocket, [localSocket, this](const Asio::Code& code)
		{
			do
			{
				if (code == asio::error::operation_aborted)
				{
					return;
				}
				if (code.value() != Asio::OK)
				{
					delete localSocket;
					CONSOLE_LOG_ERROR("{}", code.message())
					break;
				}
				Asio::Context & io = this->mApp->GetContext();
				asio::post(io, [this, localSocket]() { this->OnAcceptor(localSocket); });
			}
			while (false);
			asio::post(this->mExecutor, [this]() { this->StartAcceptor(); });
		});
	}

	void LocalNetComponent::OnAcceptor(LocalSocket* localSocket)
	{

	}
}