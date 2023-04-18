#include"TcpListenerComponent.h"
#include"Network/Tcp/SocketProxy.h"
#include"Rpc/Method/MethodProxy.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Component/ThreadComponent.h"
#include"Entity/Unit/App.h"
#include"Network/Tcp/Asio.h"
#include "Timer/Timer/ElapsedTimer.h"

namespace Tendo
{
	TcpListenerComponent::TcpListenerComponent()
    {
        this->mListenCount = 0;
        this->mBindAcceptor = nullptr;
        this->mThreadComponent = nullptr;
    }

	TcpListenerComponent::~TcpListenerComponent()
	{
		Asio::Code err;
		this->mBindAcceptor->close(err);
	}

    bool TcpListenerComponent::StopListen()
    {
		if(this->mBindAcceptor != nullptr)
		{
			asio::error_code code;
			this->mBindAcceptor->close(code);
			if (code)
			{
				CONSOLE_LOG_ERROR(code.message());
				return false;
			}
		}
        return true;
    }

	bool TcpListenerComponent::StartListen(const char * name)
	{
		unsigned short port = 0;
		this->mThreadComponent = this->GetComponent<ThreadComponent>();
		if (!ServerConfig::Inst()->GetListen(name, this->mNet, port))
		{
			LOG_ERROR("not find listen config " << name);
			return false;
		}
		this->mListenPort = 0;
#ifdef __DEBUG__
		ElapsedTimer elapsedTimer;
#endif
		std::mutex mutex;
		bool IsListen = false;
		std::chrono::milliseconds ms(2);
		std::condition_variable variable;
		Asio::Context& io = this->mThreadComponent->GetContext();
		std::shared_ptr<Asio::Timer> timer(new Asio::Timer(io, ms));
		timer->async_wait([this, port, &io, &mutex, &variable, &IsListen](const asio::error_code & ec)
		{
			std::unique_lock<std::mutex> lock(mutex);
			try
			{
				Asio::EndPoint ep(asio::ip::address_v4(), port);
				this->mBindAcceptor = std::make_unique<Asio::Acceptor>(io);

				this->mBindAcceptor->open(ep.protocol());
				this->mBindAcceptor->bind(ep);
				this->mBindAcceptor->listen();
				io.post(std::bind(&TcpListenerComponent::ListenConnect, this));
				IsListen = true;
			}
			catch (std::system_error& err)
			{
				IsListen = false;
				this->mListenPort = 0;
				asio::error_code code;
				this->mBindAcceptor->close(code);
				CONSOLE_LOG_ERROR(fmt::format("{0}  listen [{1}] failure {2}", this->GetName(), port, err.what()));
			}
			variable.notify_one();
		});
		std::unique_lock<std::mutex> lock(mutex);
		variable.wait(lock, [this, &IsListen]() {return IsListen;; });
#ifdef __DEBUG__
		if(IsListen)
		{
			LOG_INFO(this->GetName() << " listen [" << port << "] successful use " << elapsedTimer.GetMs() << " ms");
			return true;
		}
#endif
		return IsListen;
	}
	void TcpListenerComponent::ListenConnect()
	{
        std::shared_ptr<Tcp::SocketProxy> socketProxy
            = this->mThreadComponent->CreateSocket(this->mNet);
		this->mBindAcceptor->async_accept(socketProxy->GetSocket(),
			[this, socketProxy](const asio::error_code & code)
		{
			Asio::Context & main = this->mApp->MainThread();
			if(code == asio::error::operation_aborted) //强制取消
            {
                this->mBindAcceptor = nullptr;
				main.post(std::bind(&TcpListenerComponent::OnStopListen, this));
				CONSOLE_LOG_ERROR("close listen " << this->GetName() << " [" << this->mListenPort << "]");
				return;
            }
			if (code)
			{
                socketProxy->Close();
				CONSOLE_LOG_FATAL(this->GetName() << " " << code.message());
			}
			else
			{
                socketProxy->Init();
                this->mListenCount++;
				main.post(std::bind(&TcpListenerComponent::OnListen, this, socketProxy));
				CONSOLE_LOG_DEBUG(socketProxy->GetAddress() << " connect to " << this->GetName());
            }
			this->ListenConnect();
		});
	}
}
