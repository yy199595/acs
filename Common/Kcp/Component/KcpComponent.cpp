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
		udp::socket* socket = (udp::socket*)user;
		//auto endPoint = socket->local_endpoint();
		try
		{
			asio::ip::udp::endpoint endpoint(asio::ip::address_v4::from_string("127.0.0.1"), 1234);
			CONSOLE_LOG_DEBUG("send udp message to " << endpoint.port());
			size_t size = socket->send_to(asio::buffer(buf, len), endpoint);
			CONSOLE_LOG_DEBUG("send udp message successful " << "   " << std::string(buf, size));
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
				ikcpcb* kcp = ikcp_create(0x11223355, &socket);

				// Set the output function for KCP
				kcp->output = KcpComponent::KcpSendCallback;
				kcp->user = &socket;
				// Set the interval for KCP
				ikcp_nodelay(kcp, 1, 10, 2, 1);

				// Send a message to the server
				std::string message = "Hello, server!";
				asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), 1234);
				
				udp::socket socket(io_service, endpoint);
				socket.bind(endpoint);
				while (true)
				{
					asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), 1234);
					size_t size = socket.send_to(asio::buffer(message.c_str(), message.size()), endpoint);
					CONSOLE_LOG_DEBUG("send udp message successful " << "   " << message);


					//ikcp_send(kcp, message.c_str(), message.size());

					////// Update KCP
					//ikcp_update(kcp, time(NULL));
					//io_service.poll();
					std::this_thread::sleep_for(std::chrono::seconds(1));

				}
			});
		t->detach();
		return true;
	}

	bool KcpComponent::StartListen(const char* name)
	{
		unsigned short port = 0;
		if(!ServerConfig::Inst()->GetListen(name, port))
		{
			LOG_ERROR("not find listen config " << name);
			return false;
		}

		try
		{
			std::thread* t = new std::thread([port]()
				{
					std::this_thread::sleep_for(std::chrono::seconds(5));
					asio::io_service io_service;
					asio::io_service::work work(io_service);

					// Create a UDP socket
					

					// Create a UDP endpoint
					asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), 1234);

					// Bind the socket to the endpoint
					/*socket.open(endpoint.protocol());
					socket.bind(endpoint);*/

					asio::ip::udp::socket socket(io_service, endpoint);

					CONSOLE_LOG_INFO("listen udp message : " << endpoint.port());

					// Create a KCP object
					ikcpcb* kcp = ikcp_create(0x11223344, nullptr);

					// Set the output function for KCP
					kcp->output = KcpComponent::KcpSendCallback;

					// Set the interval for KCP
					ikcp_nodelay(kcp, 1, 10, 2, 1);

					// Receive a message from a client
					asio::ip::udp::endpoint client_endpoint;
					std::array<char, 1024> recv_buffer;
					while (true)
					{
						io_service.poll();
						size_t size = socket.receive_from(asio::buffer(recv_buffer), client_endpoint);

						CONSOLE_LOG_DEBUG("receive udp message : " << std::string(recv_buffer.data(), recv_buffer.size()));
						// Set the user data for KCP
						kcp->user = &socket;

						// Update KCP
						ikcp_input(kcp, recv_buffer.data(), recv_buffer.size());

						ikcp_update(kcp, time(NULL));
						std::this_thread::sleep_for(std::chrono::seconds(1));
					}
					return 0;
				});
			t->detach();
			return true;
		}
		catch (std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << std::endl;
			return false;
		}
		return true;
	}
}