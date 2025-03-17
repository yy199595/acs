//
// Created by zmhy0073 on 2022/1/15.
//
#include"Client.h"
#include"Log/Common/Debug.h"
#include"Util/Tools/String.h"
#include"Proto/Message/IProto.h"
#include "Util/Tools/Math.h"

namespace tcp
{
	Client::Client(size_t maxCount)
			: mMaxCount(maxCount)
	{
		this->mConnectCount = 0;
		this->mIsConnected = false;
	}

	Client::Client(Socket* socket, size_t count)
			: mMaxCount(count)
	{
		this->mConnectCount = 0;
		this->mIsConnected = false;
		this->mSocket.reset(socket);
	}

	void Client::SetSocket(Socket* socket)
	{
		this->mConnectCount = 0;
		this->ClearSendStream();
		this->ClearRecvStream();
		this->mSocket.reset(socket);
	}

	void Client::ClearBuffer()
	{
		this->ClearSendStream();
		this->ClearRecvStream();
	}

	void Client::Connect(int timeout)
	{
		if(this->mIsConnected)
		{
			return;
		}

		Asio::Code code;
		this->mConnectCount++;
		this->ClearRecvStream();
		this->ClearSendStream();
		this->mIsConnected = true;
		unsigned short port = this->mSocket->GetPort();
		const std::string& ip = this->mSocket->GetIp();
		Asio::Address address = asio::ip::make_address(ip, code);
		if (code.value() != Asio::OK)
		{
			this->mIsConnected = false;
			this->OnConnect(code, this->mConnectCount);
			return;
		}

		this->ClearBuffer();
		this->mSocket->MakeNewSocket();
		Asio::EndPoint endPoint(address, port);
		Asio::Socket& sock = this->mSocket->Get();
		this->StartTimer(timeout, tcp::timeout::connect);
		std::shared_ptr<Client> self = this->shared_from_this();
		sock.async_connect(endPoint, [self, timeout](const Asio::Code& code)
		{
			if(timeout > 0)
			{
				self->StopTimer();
			}
			self->mIsConnected = false;
#ifdef __ENABLE_OPEN_SSL__
			if (self->mSocket != nullptr && self->mSocket->IsOpenSsl())
			{
				self->mSocket->SslSocket().async_handshake(asio::ssl::stream_base::client,
						[self](const asio::error_code& err)
						{
							self->OnConnect(err, self->mConnectCount);
						});
				return;
			}
#endif
			if (code.value() == Asio::OK)
			{
				self->mConnectCount = 0;
			}
			self->OnConnect(code, self->mConnectCount);
		});
	}

	void Client::Connect(const std::string& host, const std::string& port, int timeout)
	{
		if (help::Str::IsIpAddress(host))
		{
			int connectPort = 0;
			if (!help::Math::ToNumber(port, connectPort))
			{
				Asio::Code code(asio::error::address_family_not_supported);
				this->OnConnect(code, 1);
				return;
			}
			this->mSocket->Init(host, connectPort);
			this->Connect(timeout);
			return;
		}
		this->mConnectCount++;
		Asio::Socket& sock = this->mSocket->Get();
		const Asio::Executor& executor = sock.get_executor();

		Asio::Code code;
		Asio::Resolver resolver(executor);
		Asio::ResolverQuery query(host, port);
		Asio::Resolver::iterator iterator = resolver.resolve(query, code);
		if (code.value() != Asio::OK)
		{
			this->OnConnect(code, 1);
			return;
		}
		this->StartTimer(timeout, tcp::timeout::connect);
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::async_connect(sock, iterator, [iterator, self, timeout]
				(const asio::error_code& code, const Asio::Resolver::iterator& iter)
		{
			if(timeout > 0)
			{
				self->StopTimer();
			}
#ifdef __ENABLE_OPEN_SSL__
			if (self->mSocket && self->mSocket->IsOpenSsl())
			{
				self->mSocket->SslSocket().async_handshake(asio::ssl::stream_base::client,
						[self](const asio::error_code& err)
						{
							//CONSOLE_LOG_FATAL("{}", err.message())
							self->OnConnect(err, self->mConnectCount);
						});
				return;
			}
#endif
			if(code.value() == Asio::OK)
			{
				self->mConnectCount = 0;
			}
			self->OnConnect(code, self->mConnectCount);
		});
	}

	void Client::ReadAll(int timeout)
	{
		this->StartTimer(timeout, tcp::timeout::read);
		std::shared_ptr<Client> self = this->shared_from_this();
		auto callBack = [self, timeout](const asio::error_code& code, size_t size)
		{
			if(timeout > 0)
			{
				self->StopTimer();
			}
			std::istream ss(&self->mRecvBuffer);
			if (code.value() == Asio::OK)
			{
				self->OnReceiveMessage(ss, size, code);
				return;
			}
			if (self->mRecvBuffer.size() > 0)
			{
				size = self->mRecvBuffer.size();
				self->OnReceiveMessage(ss, size, code);
			}
			if (code != asio::error::operation_aborted)
			{
				self->OnReadError(code);
			}
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

	void Client::ReadLine(int timeout)
	{
		this->StartTimer(timeout, tcp::timeout::read);
		std::shared_ptr<Client> self = this->shared_from_this();
		auto callback = [self, timeout](const Asio::Code& code, size_t size)
		{
			if(timeout > 0)
			{
				self->StopTimer();
			}
			std::istream ss(&self->mRecvBuffer);
			if (code.value() == Asio::OK)
			{
				self->OnReceiveLine(ss, size);
				return;
			}
			if (self->mRecvBuffer.size() > 0)
			{
				size = self->mRecvBuffer.size();
				self->OnReceiveLine(ss, size);
				return;
			}
			if (code != asio::error::operation_aborted)
			{
				self->OnReadError(code);
			}
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

	void Client::ReadSome(int timeout)
	{
		std::istream ss(&this->mRecvBuffer);
		if (this->mRecvBuffer.size() > 0)
		{
			asio::error_code code;
			size_t size = this->mRecvBuffer.size();
			this->OnReceiveMessage(ss, size, code);
			return;
		}
		this->StartTimer(timeout, tcp::timeout::read);
		std::shared_ptr<Client> self = this->shared_from_this();
		auto callBack = [self, timeout](const asio::error_code& code, size_t size)
		{
			if(timeout > 0)
			{
				self->StopTimer();
			}
			std::istream ss(&self->mRecvBuffer);
			if (code.value() == Asio::OK)
			{
				self->OnReceiveMessage(ss, size, code);
				return;
			}
			if (self->mRecvBuffer.size() > 0)
			{
				size = self->mRecvBuffer.size();
				self->OnReceiveMessage(ss, size, code);
			}
			if (code != asio::error::operation_aborted)
			{
				self->OnReadError(code);
			}
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

	void Client::ReadLength(size_t length, int timeout)
	{
		if (length <= 0)
		{
			//CONSOLE_LOG_FATAL("length = {}", length);
			this->OnReadError(std::make_error_code(std::errc::bad_message));
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
			asio::error_code code;
			this->OnReceiveMessage(ss, length, code);
			return;
		}

		this->StartTimer(timeout, tcp::timeout::read);
		std::shared_ptr<Client> self = this->shared_from_this();
		auto callBack = [self, length, timeout](const asio::error_code& code, size_t size)
		{
			if(timeout > 0)
			{
				self->StopTimer();
			}
			std::istream ss(&self->mRecvBuffer);
			if (code.value() == Asio::OK)
			{
				self->OnReceiveMessage(ss, size, code);
				return;
			}
			//this->OnReceiveMessage(ss, size);
			if (code != asio::error::operation_aborted)
			{
				self->OnReadError(code);
			}
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

	void Client::Write(tcp::IProto& message, int timeout)
	{
		std::ostream os(&this->mSendBuffer);
		int length = message.OnSendMessage(os);
		this->StartTimer(timeout, tcp::timeout::send);
		std::shared_ptr<Client> self = this->shared_from_this();
		auto callBack = [self, length, &message](const Asio::Code& code, size_t size)
		{
			if (code.value() != Asio::OK)
			{
				self->OnSendMessage(code);
				return;
			}
			if (length > 0)
			{
				self->Write(message, 0);
				return;
			}
			self->OnSendMessage(size);
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
	void Client::ClearRecvStream()
	{
		if (this->mRecvBuffer.size() > 0)
		{
			std::iostream os(&this->mRecvBuffer);
			os.ignore((int)this->mRecvBuffer.size());
		}
	}

	bool Client::ConnectSync(Asio::Code& code)
	{
		if (!this->mSocket->IsClient())
		{
			return false;
		}
		try
		{
			unsigned short port = this->mSocket->GetPort();
			const std::string& ip = this->mSocket->GetIp();
			Asio::EndPoint endPoint(asio::ip::make_address(ip), port);
			this->mSocket->Get().connect(endPoint);
			return true;
		}
		catch (const std::system_error & err)
		{
			code = err.code();
			return false;
		}
	}

	bool Client::ConnectSync(const std::string& host, const std::string& port)
	{
		try
		{
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
					this->mSocket->Get().connect(endPoint);
				}
				return true;
			}
			Asio::Socket& sock = this->mSocket->Get();
			const Asio::Executor& executor = sock.get_executor();
			Asio::Resolver resolver(executor);
			Asio::ResolverQuery query(host, port);
			Asio::Resolver::iterator iterator = resolver.resolve(query);
			asio::connect(sock, iterator);
#ifdef __ENABLE_OPEN_SSL__
			if(this->mSocket->IsOpenSsl())
			{
				this->mSocket->SslSocket().handshake(asio::ssl::stream_base::client);
			}
#endif
			return true;
		}
		catch (const std::system_error & err)
		{
			return false;
		}
	}

	void Client::ClearSendStream()
	{
		if (this->mSendBuffer.size() > 0)
		{
			std::iostream os(&this->mSendBuffer);
			os.ignore((int)this->mSendBuffer.size());
		}
	}

	bool Client::RecvSomeSync(size_t& size)
	{
		try
		{
			if (this->mRecvBuffer.size() > 0)
			{
				size = this->mRecvBuffer.size();
				return true;
			}
#ifdef __ENABLE_OPEN_SSL__
			if (this->mSocket->IsOpenSsl())
			{
				Asio::ssl::Socket& sock = this->mSocket->SslSocket();
				size = asio::read(sock, this->mRecvBuffer, asio::transfer_at_least(1));
				return true;
			}
#endif
			Asio::Socket& sock = this->mSocket->Get();
			size = asio::read(sock, this->mRecvBuffer, asio::transfer_at_least(1));
			return true;
		}
		catch (asio::error_code & code)
		{
			this->OnReadError(code);
			return false;
		}
	}

	bool Client::RecvSync(size_t length, size_t& size)
	{
		try
		{
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
		catch (asio::system_error & err)
		{
			if(err.code() == asio::error::eof)
			{
				size = this->mRecvBuffer.size();
			}
			if(size < length)
			{
				this->OnReadError(err.code());
			}
			return size >= length;
		}
	}

	bool Client::RecvLineSync(size_t& size)
	{
		try
		{
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
		catch (asio::system_error & code)
		{
			this->OnReadError(code.code());
			return false;
		}
	}

	bool Client::SendSync(tcp::IProto& message)
	{
		try
		{
			int length = 0;
			do
			{
				size_t sendCount = 0;
				std::ostream os(&this->mSendBuffer);
				length = message.OnSendMessage(os);
#ifdef __ENABLE_OPEN_SSL__
				if (this->mSocket->IsOpenSsl())
				{
					Asio::ssl::Socket& sock = this->mSocket->SslSocket();
					sendCount = asio::write(sock, this->mSendBuffer);
				}
				else
				{
					sendCount = asio::write(this->mSocket->Get(), this->mSendBuffer);
				}
#else
				sendCount = asio::write(this->mSocket->Get(), this->mSendBuffer);
#endif
				if(sendCount <= 0)
				{
					return false;
				}
			} while (length > 0);
			return true;
		}
		catch (std::system_error& error)
		{
			return false;
		}
	}

	bool Client::SendSync(const char* message, size_t size)
	{
		try
		{
#ifdef __ENABLE_OPEN_SSL__
			if (this->mSocket->IsOpenSsl())
			{
				Asio::ssl::Socket& sock = this->mSocket->SslSocket();
				return asio::write(sock, asio::buffer(message, size)) == size;
			}
			return asio::write(this->mSocket->Get(), asio::buffer(message, size)) == size;
#else
			return asio::write(this->mSocket->Get(), asio::buffer(message, size)) == size;
#endif
			return true;
		}
		catch (std::system_error& error)
		{
			this->OnSendMessage(error.code());
			return false;
		}
	}

	void Client::StopTimer()
	{
		if (this->mTimer != nullptr)
		{
			asio::error_code code;
			this->mTimer->cancel(code);
			this->mTimer.reset();
		}
	}

	void Client::StopUpdate()
	{
		if (this->mUpdateTimer != nullptr)
		{
			asio::error_code code;
			this->mUpdateTimer->cancel(code);
			this->mUpdateTimer.reset();
		}
	}

	void Client::StartUpdate(int timeout)
	{
		if (this->mUpdateTimer == nullptr)
		{
			Asio::Socket& sock = this->mSocket->Get();
			const Asio::Executor& executor = sock.get_executor();
			this->mUpdateTimer = std::make_unique<Asio::Timer>(executor);
		}
		std::shared_ptr<Client> self = this->shared_from_this();
		this->mUpdateTimer->expires_after(std::chrono::seconds(timeout));
		this->mUpdateTimer->async_wait([self, timeout](const Asio::Code& code)
		{
			if (code.value() == Asio::OK)
			{
				self->OnUpdate();
			}
			if (code == asio::error::operation_aborted)
			{
				return;
			}
			self->StartUpdate(timeout);
		});
	}

	void Client::StartTimer(int timeout, tcp::timeout flag)
	{
		if (timeout <= 0) return;
		if(this->mTimer == nullptr)
		{
			Asio::Socket& sock = this->mSocket->Get();
			const Asio::Executor& executor = sock.get_executor();
			this->mTimer = std::make_unique<Asio::Timer>(executor);
		}
		std::shared_ptr<Client> self = this->shared_from_this();
		this->mTimer->expires_after(std::chrono::seconds(timeout));
		this->mTimer->async_wait([self, timeout, flag](const Asio::Code& code)
		{
			if (code.value() == Asio::OK)
			{
				Asio::Code code(asio::error::timed_out);
				switch (flag)
				{
					case tcp::timeout::connect:
						self->OnConnect(code, 1);
						break;
					case tcp::timeout::read:
						self->OnReadError(code);
						break;
					case tcp::timeout::send:
						self->OnSendMessage(code);
						break;
				}
			}
		});
	};
}