//
// Created by MyPC on 2023/4/7.
//

#include"KcpComponent.h"

#include"asio.hpp"
#include<thread>
#include<iostream>
#include"Entity/App/App.h"
#include"Kcp/Common/ikcp.h"
using asio::ip::udp;
namespace Tendo
{
	
	int KcpComponent::KcpSendCallback(const char * buf, int len, ikcpcb * kcp, void * user)
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

	void KcpComponent::OnSecondUpdate(int tick)
	{
		
		
	}

	bool KcpComponent::LateAwake()
	{
		this->StartListen("udp");
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
				kcp->output = KcpComponent::KcpSendCallback;
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

	void KcpComponent::Update(asio::io_service& io, long long time)
	{

	}

	bool KcpComponent::StartListen(const char* name)
	{
		unsigned short port = 0;
		if(!ServerConfig::Inst()->GetListen(name, port))
		{
			LOG_ERROR("not find listen config " << name);
			return false;
		}
		asio::io_service & main = this->mApp->MainThread();
		this->mThread = new std::thread([&main, this]()
		{
			asio::io_context io;
			bool startReceive = true;
			asio::io_service::work work(io);
			std::chrono::milliseconds ms(1);
			std::array<char, 56635> receiveBuffer;
			asio::ip::udp::endpoint clientEndpoint;
			asio::ip::udp::socket udpSocket(io, udp::endpoint(udp::v4(), 1234));
			ikcpcb* kcp = ikcp_create(0x11223344, &udpSocket);
			while (!io.stopped())
			{
				io.poll();
				if(startReceive)
				{
					startReceive = false;
					udpSocket.async_receive_from(asio::buffer(receiveBuffer), clientEndpoint, 0,
						[&clientEndpoint, &startReceive, &receiveBuffer](const asio::error_code& code, size_t size)
						{
							if (code)
							{
								CONSOLE_LOG_ERROR(code.message());
							}
							startReceive = true;
						});
				}
				long long time = Helper::Time::NowMicTime();

				this->Update(io, time);
				ikcp_update(kcp, (IUINT32)time);
				std::this_thread::sleep_for(ms);
			}
		});
		this->mThread->detach();
//		try
//		{
//			std::thread* t = new std::thread([port]()
//				{
//					std::this_thread::sleep_for(std::chrono::seconds(5));
//					asio::io_service io_service;
//					asio::io_service::work work(io_service);
//
//					// Create a UDP socket
//
//
//					asio::ip::udp::socket socket(io_service, udp::endpoint(udp::v4(), 1234));
//
//					ikcpcb* kcp = ikcp_create(0x11223344, nullptr);
//
//					// Set the output function for KCP
//					kcp->output = KcpComponent::KcpSendCallback;
//
//					// Set the interval for KCP
//					ikcp_nodelay(kcp, 1, 10, 2, 1);
//
//					// Receive a message from a client
//					asio::ip::udp::endpoint client_endpoint;
//					std::array<char, 1024> recv_buffer;
//					while (!io_service.stopped())
//					{
//						socket.receive_from(asio::buffer(recv_buffer), client_endpoint);
//						CONSOLE_LOG_INFO("receive udp message : " << client_endpoint.address().to_string());
//						// Set the user data for KCP
//						kcp->user = &socket;
//						ikcp_input(kcp, recv_buffer.data(), recv_buffer.size());
//
//						// Update KCP
//
//						ikcp_update(kcp, time(NULL));
//						std::this_thread::sleep_for(std::chrono::seconds(1));
//					}
//					return 0;
//				});
//			t->detach();
//			return true;
//		}
//		catch (std::exception& e)
//		{
//			std::cerr << "Exception: " << e.what() << std::endl;
//			return false;
//		}
		return true;
	}
}