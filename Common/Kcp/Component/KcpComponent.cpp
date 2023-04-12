//
// Created by MyPC on 2023/4/7.
//

#include"KcpComponent.h"

#include"asio.hpp"
#include<thread>
#include<iostream>
#include"Entity/Unit/App.h"
#include"Kcp/Common/ikcp.h"
using asio::ip::udp;
namespace Tendo
{
	
	int KcpComponent::OnServerSend(const char * buf, int len, ikcpcb * kcp, void * user)
	{
		asio::error_code ec;
		udp::socket* socket = (udp::socket*)kcp->user;
		udp::endpoint target(asio::ip::make_address("127.0.0.1"), 1234);

		//auto endPoint = socket->local_endpoint();
		try
		{
			size_t size = socket->send_to(asio::buffer(buf, len),  target, 0, ec);
			if(!ec)
			{
				CONSOLE_LOG_DEBUG("send udp message successful size=" << size);
				return 1;
			}
			CONSOLE_LOG_ERROR(ec.message());
		}
		catch (const std::exception& e)
		{
			CONSOLE_LOG_FATAL(e.what());
		}
	
		return 0;
	}

	int KcpComponent::OnClientSend(const char* buf, int len, ikcpcb* kcp, void* user)
	{
		asio::error_code ec;
		udp::socket* socket = (udp::socket*)kcp->user;
		udp::endpoint target(asio::ip::make_address("127.0.0.1"), 1234);

		//auto endPoint = socket->local_endpoint();
		try
		{
			size_t size = socket->send_to(asio::buffer(buf, len), target, 0, ec);
			if (!ec)
			{
				CONSOLE_LOG_DEBUG("send udp message successful size=" << size);
				return 1;
			}
			CONSOLE_LOG_ERROR(ec.message());
		}
		catch (const std::exception& e)
		{
			CONSOLE_LOG_FATAL(e.what());
		}

		return 0;
	}

	KcpComponent::KcpComponent()
	{
		this->mThread = nullptr;
		this->mServerSocket = nullptr;
		this->mContext = nullptr;
		this->mKcpServer = nullptr;
		this->mPort = 0;
	}

	void KcpComponent::OnSecondUpdate(int tick)
	{
		
		
	}

	bool KcpComponent::LateAwake()
	{
		this->StartListen("kcp");
		std::thread* t = new std::thread([]()
			{
				std::this_thread::sleep_for(std::chrono::seconds(5));
				asio::io_service io_service;
				asio::io_service::work work(io_service);

				// Create a UDP socket
				

				// Create a UDP endpoint


				// Create a KCP object
				udp::socket udpSocket(io_service, udp::endpoint(udp::v4(), 0));
				udp::endpoint target(asio::ip::make_address("127.0.0.1"), 1234);

				ikcpcb* kcp = ikcp_create(1, (void *)&udpSocket);
				kcp->output = KcpComponent::OnClientSend;
				ikcp_nodelay(kcp, 1, 10, 2, 1);

				// Send a message to the server
				std::string message = "Hello, server!";
				while (!io_service.stopped())
				{
					//asio::error_code ec;
					//socket.send_to(asio::buffer(message.c_str(), message.size()), target, 0, ec);
					ikcp_send(kcp, message.c_str(), message.size());

					////// Update KCP
					ikcp_update(kcp, time(NULL));
					//io_service.poll();
					std::this_thread::sleep_for(std::chrono::seconds(1));
				}
			});
		t->detach();
		return true;
	}

	void KcpComponent::Main()
	{
		asio::error_code ec;
		std::chrono::milliseconds ms(1);
		this->mContext = new Asio::Context(1);
		Asio::ContextWork work(*this->mContext);
		this->mKcpServer = ikcp_create(1, this);
		this->mKcpServer->output = KcpComponent::OnServerSend;
		this->mServerSocket = new udp::socket(*this->mContext, udp::endpoint(udp::v4(), 1234));

		this->mContext->post(std::bind(&KcpComponent::Receive, this));
		while (!this->mContext->stopped())
		{
			this->mContext->poll(ec);
			this->Update(*this->mContext);
			std::this_thread::sleep_for(ms);
		}
	}

	void KcpComponent::Update(Asio::Context& context)
	{

	}

	void KcpComponent::Receive()
	{
		this->mServerSocket->async_receive_from(asio::buffer(this->mReceiveBuffer), this->mClientEndpoint, 0,
			[this](const asio::error_code& code, size_t size)
			{
				if (code)
				{
					CONSOLE_LOG_ERROR(code.message());
				}
				// ���յ���Ϣ
				std::string ip = this->mClientEndpoint.address().to_string();
				std::string address = fmt::format("{0}:{1}", ip, this->mClientEndpoint.port());

				ikcpcb* kcpClient = nullptr;
				auto iter = this->mKcpClients.begin();
				if (iter == this->mKcpClients.end()) //������
				{
					udp::endpoint* endPoint = new udp::endpoint(this->mClientEndpoint);
					{
						kcpClient = ikcp_create(1, endPoint);
						kcpClient->output = KcpComponent::OnClientSend;
						this->mKcpClients.emplace(address, kcpClient);
					}
				}
				else
				{
					kcpClient = iter->second;
				}

				ikcp_send(kcpClient, this->mReceiveBuffer.data(), size);
				this->mContext->post(std::bind(&KcpComponent::Receive, this));
				ikcp_input(this->mKcpServer, this->mReceiveBuffer.data(), size);
			});
	}

	

	bool KcpComponent::StartListen(const char* name)
	{
		if(!ServerConfig::Inst()->GetListen(name, this->mPort))
		{
			LOG_ERROR("not find listen config " << name);
			return false;
		}
		if (this->mThread != nullptr)
		{
			return false;
		}
		this->mThread = new std::thread(std::bind(&KcpComponent::Main, this));
		this->mThread->detach();
		return true;
		/*this->mThread = new std::thread([this]()
		{
			asio::io_context io;
			bool startReceive = true;
			asio::io_service::work work(io);
			std::chrono::milliseconds ms(1);
			std::array<char, 56635> receiveBuffer;
			asio::ip::udp::endpoint* endPoint = new asio::ip::udp::endpoint();
			asio::ip::udp::socket udpSocket(io, udp::endpoint(udp::v4(), 1234));
			ikcpcb* kcp = ikcp_create(0x11223344, &udpSocket);
			unsigned int index = 1;
			while (!io.stopped())
			{
				io.poll();
				if(startReceive)
				{
					startReceive = false;
					udpSocket.async_receive_from(asio::buffer(receiveBuffer), *endPoint, 0,
						[endPoint, &index, &startReceive, &receiveBuffer](const asio::error_code& code, size_t size)
						{
							if (code)
							{
								CONSOLE_LOG_ERROR(code.message());
							}
							startReceive = true;
							ikcpcb * kcpClient = ikcp_create(index++, endPoint);
							
						});
					
				}
				long long time = Helper::Time::NowMicTime();

				this->Update(io, time);
				ikcp_update(kcp, (IUINT32)time);
				std::this_thread::sleep_for(ms);
			}
		});
		this->mThread->detach();
		return true;*/
	}
}