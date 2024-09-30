//
// Created by zmhy0073 on 2022/1/15.
//
#include"TcpClient.h"
#include"Log/Common/Debug.h"
#include"Util/Tools/String.h"
#include"Proto/Message/IProto.h"
#include "Util/Tools/Math.h"

namespace tcp
{
	TcpClient::TcpClient(size_t maxCount)
			: mMaxCount(maxCount)
	{
		this->mConnectCount = 0;
	}

	TcpClient::TcpClient(Socket* socket, size_t count)
			: mMaxCount(count)
	{
		this->mConnectCount = 0;
		this->mSocket.reset(socket);
	}

	void TcpClient::SetSocket(Socket* socket)
	{
		this->mConnectCount = 0;
		this->ClearSendStream();
		this->ClearRecvStream();
		this->mSocket.reset(socket);
	}

	void TcpClient::ClearBuffer()
	{
		this->ClearSendStream();
		this->ClearRecvStream();
	}

	void TcpClient::Connect(int timeout)
	{
		Asio::Code code;
		this->mConnectCount++;
		unsigned short port = this->mSocket->GetPort();
		const std::string& ip = this->mSocket->GetIp();
		this->StartTimer(timeout, TimeoutFlag::Connect);
		Asio::Address address = asio::ip::make_address(ip, code);
		if (code.value() != Asio::OK)
		{
			CONSOLE_LOG_ERROR("[{}:{}] ", ip, port, code.message());
			return;
		}
		this->ClearBuffer();
		this->mSocket->MakeNewSocket();
		Asio::EndPoint endPoint(address, port);
		Asio::Socket& sock = this->mSocket->Get();
		sock.async_connect(endPoint, [this](const Asio::Code& code)
		{
			if (code.value() != Asio::OK)
			{
				this->OnConnect(false, 1);
				return;
			}
#ifdef __ENABLE_OPEN_SSL__
			if (this->mSocket->IsOpenSsl())
			{
				this->mSocket->SslSocket().async_handshake(asio::ssl::stream_base::client,
						[this](const asio::error_code& err)
						{
							this->OnConnect(err.value() == Asio::OK, this->mConnectCount);
						});
				return;
			}
#endif
			this->mConnectCount = 0;
			this->OnConnect(code.value() == Asio::OK, this->mConnectCount);
		});
	}

	void TcpClient::Connect(const std::string& host, const std::string& port, int timeout)
	{
		if (help::Str::IsIpAddress(host))
		{
			int connectPort = 0;
			if (!help::Math::ToNumber(port, connectPort))
			{
				this->OnConnect(false, 1);
				return;
			}
			this->mSocket->Init(host, connectPort);
			this->Connect(timeout);
			return;
		}
		Asio::Socket& sock = this->mSocket->Get();
		this->StartTimer(timeout, TimeoutFlag::Connect);
		const Asio::Executor& executor = sock.get_executor();

		Asio::Code code;
		Asio::Resolver resolver(executor);
		Asio::ResolverQuery query(host, port);
		Asio::Resolver::iterator iterator = resolver.resolve(query, code);
		if (code.value() != Asio::OK)
		{
			this->OnConnect(false, 1);
			return;
		}
		asio::async_connect(sock, iterator, [this, iterator]
				(const asio::error_code& code, const Asio::Resolver::iterator& iter)
		{
			this->mConnectCount++;
			if (code.value() != Asio::OK)
			{
				return;
			}
#ifdef __ENABLE_OPEN_SSL__
			if (this->mSocket->IsOpenSsl())
			{
				this->mSocket->SslSocket().async_handshake(asio::ssl::stream_base::client,
						[this](const asio::error_code& err)
						{
							this->OnConnect(err.value() == Asio::OK, this->mConnectCount);
						});
				return;
			}
#endif
			this->mConnectCount = 0;
			this->OnConnect(code.value() == Asio::OK, 0);
		});
	}

	void TcpClient::ReadAll(int timeout)
	{
		if (!this->mSocket->IsOpen())
		{
			Asio::Code code(asio::error::not_socket);
			this->OnReadError(code);
			return;
		}
		this->StartTimer(timeout, TimeoutFlag::ReadCount);
		auto callBack = [this](const asio::error_code& code, size_t size)
			{
				std::istream ss(&this->mRecvBuffer);
				if (code.value() == Asio::OK)
				{
					this->OnReceiveMessage(ss, size);
					return;
				}
				if (this->mRecvBuffer.size() > 0)
				{
					size = this->mRecvBuffer.size();
					this->OnReceiveMessage(ss, size);
				}
				this->OnReadError(code);
			};

#ifdef __ENABLE_OPEN_SSL__
		if (this->mSocket->IsOpenSsl())
		{
			Asio::ssl::Socket& sock = this->mSocket->SslSocket();
			asio::async_read(sock, this->mRecvBuffer, asio::transfer_all(), callBack);
			return;
		}
#endif
		Asio::Socket& sock = this->mSocket->Get();
		asio::async_read(sock, this->mRecvBuffer, asio::transfer_all(), callBack);
	}

	void TcpClient::ReadLine(int timeout)
	{
		if (!this->mSocket->IsOpen())
		{
			Asio::Code code(asio::error::not_socket);
			this->OnReadError(code);
			return;
		}
		this->StartTimer(timeout, TimeoutFlag::ReadLine);
		auto callback = [this](const Asio::Code& code, size_t size)
		{
			std::istream ss(&this->mRecvBuffer);
			if (code.value() == Asio::OK)
			{
				this->OnReceiveLine(ss, size);
				return;
			}
			if (this->mRecvBuffer.size() > 0)
			{
				size = this->mRecvBuffer.size();
				this->OnReceiveLine(ss, size);
				return;
			}
			this->OnReadError(code);
		};
#ifdef __ENABLE_OPEN_SSL__
		if (this->mSocket->IsOpenSsl())
		{
			Asio::ssl::Socket& sock = this->mSocket->SslSocket();
			asio::async_read_until(sock, this->mRecvBuffer, "\r\n", callback);
			return;
		}
#endif
		Asio::Socket& sock = this->mSocket->Get();
		asio::async_read_until(sock, this->mRecvBuffer, "\r\n", callback);
	}

	void TcpClient::ReadSome(int timeout)
	{
		if (!this->mSocket->IsOpen())
		{
			Asio::Code code(asio::error::not_socket);
			this->OnReadError(code);
			return;
		}
		std::istream ss(&this->mRecvBuffer);
		if (this->mRecvBuffer.size() > 0)
		{
			size_t size = this->mRecvBuffer.size();
			this->OnReceiveMessage(ss, size);
			return;
		}
		this->StartTimer(timeout, TimeoutFlag::ReadSome);
		auto callBack = [this](const asio::error_code& code, size_t size)
		{
			std::istream ss(&this->mRecvBuffer);
			if (code.value() == Asio::OK)
			{
				this->OnReceiveMessage(ss, size);
				return;
			}
			if (this->mRecvBuffer.size() > 0)
			{
				size = this->mRecvBuffer.size();
				this->OnReceiveMessage(ss, size);
			}
			this->OnReadError(code);
		};
#ifdef __ENABLE_OPEN_SSL__
		if (this->mSocket->IsOpenSsl())
		{
			Asio::ssl::Socket& sock = this->mSocket->SslSocket();
			//sock.async_read_some(this->mRecvBuffer.prepare(1024), callBack);
			asio::async_read(sock, this->mRecvBuffer, asio::transfer_at_least(1), callBack);
			return;
		}
#endif
		Asio::Socket& sock = this->mSocket->Get();
		//sock.async_read_some(this->mRecvBuffer.prepare(1024), callBack);
		asio::async_read(sock, this->mRecvBuffer, asio::transfer_at_least(1), callBack);
	}

	void TcpClient::ReadLength(int length, int timeout)
	{
		if (length <= 0)
		{
			CONSOLE_LOG_FATAL("length = {}", length);
			return;
		}
		if (this->mMaxCount > 0 && length >= this->mMaxCount)
		{
			this->OnReadError(std::make_error_code(std::errc::bad_message));
			return;
		}
		std::istream ss(&this->mRecvBuffer);
		if (this->mRecvBuffer.size() >= length)
		{
			this->OnReceiveMessage(ss, length);
			return;
		}
		if (!this->mSocket->IsOpen())
		{
			Asio::Code code(asio::error::not_socket);
			this->OnReadError(code);
			return;
		}

		this->StartTimer(timeout, TimeoutFlag::ReadCount);
		auto callBack = [this](const asio::error_code& code, size_t size)
		{
			std::istream ss(&this->mRecvBuffer);
			if (code.value() == Asio::OK)
			{
				this->OnReceiveMessage(ss, size);
				return;
			}
			if (this->mRecvBuffer.size() > 0)
			{
				size = this->mRecvBuffer.size();
				this->OnReceiveMessage(ss, size);
			}
			this->OnReadError(code);
		};

#ifdef __ENABLE_OPEN_SSL__
		if (this->mSocket->IsOpenSsl())
		{
			Asio::ssl::Socket& sock = this->mSocket->SslSocket();
			asio::async_read(sock, this->mRecvBuffer, asio::transfer_exactly(length), callBack);
			return;
		}
#endif
		Asio::Socket& sock = this->mSocket->Get();
		asio::async_read(sock, this->mRecvBuffer, asio::transfer_exactly(length), callBack);
	}


	void TcpClient::Write(tcp::IProto& message, int timeout)
	{
		if (!this->mSocket->IsOpen())
		{
			Asio::Code code(asio::error::not_socket);
			this->OnSendMessage(code);
			return;
		}
		std::ostream os(&this->mSendBuffer);
		this->StartTimer(timeout, TimeoutFlag::Write);
		int length = message.OnSendMessage(os);
		auto callBack = [this, length, &message](const Asio::Code& code, size_t size)
		{
			if (code.value() != Asio::OK)
			{
				this->OnSendMessage(code);
				return;
			}
			if (length > 0)
			{
				this->Write(message, 0);
				return;
			}
			this->OnSendMessage();
		};
#ifdef __ENABLE_OPEN_SSL__
		if (this->mSocket->IsOpenSsl())
		{
			Asio::ssl::Socket& sock = this->mSocket->SslSocket();
			asio::async_write(sock, this->mSendBuffer, callBack);
			return;
		}
#endif

		Asio::Socket& sock = this->mSocket->Get();
		asio::async_write(sock, this->mSendBuffer, callBack);
	}
}

namespace tcp
{
	void TcpClient::ClearRecvStream()
	{
		if (this->mRecvBuffer.size() > 0)
		{
			std::iostream os(&this->mRecvBuffer);
			os.ignore((int)this->mRecvBuffer.size());
		}
	}

	bool TcpClient::ConnectSync(Asio::Code& code)
	{
		if (!this->mSocket->IsClient())
		{
			return false;
		}
		unsigned short port = this->mSocket->GetPort();
		const std::string& ip = this->mSocket->GetIp();
		Asio::Address address = asio::ip::make_address(ip);
		Asio::EndPoint endPoint(address, port);
		this->mSocket->Get().connect(endPoint, code);
		return code.value() == Asio::OK;
	}

	bool TcpClient::ConnectSync(const std::string& host, const std::string& port)
	{
		Asio::Code code;
		int connPort = 0;
		if (help::Str::IsIpAddress(host))
		{
			if (!help::Math::ToNumber(port, connPort))
			{
				return false;
			}
			Asio::Address address = asio::ip::make_address(host);
			{
				Asio::EndPoint endPoint(address, connPort);
				this->mSocket->Get().connect(endPoint, code);
			}
			return code.value() == Asio::OK;
		}
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& executor = sock.get_executor();
		Asio::Resolver resolver(executor);
		Asio::ResolverQuery query(host, port);
		Asio::Resolver::iterator iterator = resolver.resolve(query, code);
		if (code.value() != Asio::OK)
		{
			return false;
		}
		asio::connect(sock, iterator, code);
		if(this->mSocket->IsOpenSsl())
		{
			this->mSocket->SslSocket().handshake(asio::ssl::stream_base::client, code);
		}
		return code.value() == Asio::OK;
	}

	void TcpClient::ClearSendStream()
	{
		if (this->mSendBuffer.size() > 0)
		{
			std::iostream os(&this->mSendBuffer);
			os.ignore((int)this->mSendBuffer.size());
		}
	}

	bool TcpClient::RecvSync(int length, size_t& size)
	{
		try
		{
			if (!this->mSocket->IsOpen())
			{
				return false;
			}
			if (this->mRecvBuffer.size() >= length)
			{
				size = length;
				return true;
			}
#ifdef __ENABLE_OPEN_SSL__
			if (this->mSocket->IsOpenSsl())
			{
				Asio::ssl::Socket& sock = this->mSocket->SslSocket();
				size = asio::read(sock, this->mRecvBuffer, asio::transfer_exactly(length));
				return true;
			}
#endif
			Asio::Socket& sock = this->mSocket->Get();
			size = asio::read(sock, this->mRecvBuffer, asio::transfer_exactly(length));
			return true;
		}
		catch (asio::system_error& code)
		{
			return false;
		}
	}

	bool TcpClient::RecvLineSync(size_t& size)
	{
		try
		{
			if (!this->mSocket->IsOpen())
			{
				return false;
			}
#ifdef __ENABLE_OPEN_SSL__
			if (this->mSocket->IsOpenSsl())
			{
				Asio::ssl::Socket& sock = this->mSocket->SslSocket();
				size = asio::read_until(sock, this->mRecvBuffer, "\r\n");
				return true;
			}
#endif
			Asio::Socket& sock = this->mSocket->Get();
			size = asio::read_until(sock, this->mRecvBuffer, "\r\n");
			return true;
		}
		catch (asio::system_error& code)
		{
			return false;
		}
	}

	bool TcpClient::SendSync(tcp::IProto& message)
	{
		try
		{
			int length = 0;
			do
			{
				if (!this->mSocket->IsOpen())
				{
					return false;
				}
				std::ostream os(&this->mSendBuffer);
				length = message.OnSendMessage(os);
#ifdef __ENABLE_OPEN_SSL__
				if (this->mSocket->IsOpenSsl())
				{
					Asio::ssl::Socket& sock = this->mSocket->SslSocket();
					asio::write(sock, this->mSendBuffer);
				}
				else
				{
					asio::write(this->mSocket->Get(), this->mSendBuffer);
				}
#else
				asio::write(this->mSocket->Get(), this->mSendBuffer);
#endif
			} while (length > 0);
			return true;
		}
		catch (std::system_error& error)
		{
			//CONSOLE_LOG_ERROR("sync send {} err:{}", message->ToString(), error.what());
			return false;
		}
	}

	void TcpClient::StopTimer()
	{
		if (this->mTimer != nullptr)
		{
			asio::error_code code;
			this->mTimer->cancel(code);
			this->mTimer = nullptr;
		}
	}

	void TcpClient::StopUpdate()
	{
		if (this->mUpdateTimer != nullptr)
		{
			asio::error_code code;
			this->mUpdateTimer->cancel(code);
			this->mUpdateTimer = nullptr;
		}
	}

	void TcpClient::StartUpdate(int timeout)
	{
		if (this->mUpdateTimer == nullptr)
		{
			Asio::Socket& sock = this->mSocket->Get();
			const Asio::Executor& executor = sock.get_executor();
			this->mUpdateTimer = std::make_unique<Asio::Timer>(executor);
		}
		this->mUpdateTimer->expires_after(std::chrono::seconds(timeout));
		this->mUpdateTimer->async_wait([this, timeout](const Asio::Code& code)
		{
			if (code.value() == Asio::OK)
			{
				this->OnUpdate();
			}
			if (code == asio::error::operation_aborted)
			{
				return;
			}
			this->StartUpdate(timeout);
		});
	}

	void TcpClient::StartTimer(int timeout, TimeoutFlag flag)
	{
		if (timeout <= 0) return;
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& executor = sock.get_executor();
		this->mTimer = std::make_unique<Asio::Timer>(executor);
		this->mTimer->expires_after(std::chrono::seconds(timeout));
		this->mTimer->async_wait([this, timeout, flag](const Asio::Code& code)
		{
			if (code.value() == Asio::OK)
			{
				this->OnTimeout(flag);
			}
		});
	};
}