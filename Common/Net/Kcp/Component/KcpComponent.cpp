//
// Created by MyPC on 2023/4/7.
//

#include"KcpComponent.h"

#include"asio.hpp"
#include<thread>
#include<iostream>
#include"Entity/Actor/App.h"
#include"Kcp/Common/ikcp.h"
#include "Server/Component/ThreadComponent.h"

using asio::ip::udp;
namespace acs
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
				CONSOLE_LOG_DEBUG("send udp message successful size={}", size);
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
				CONSOLE_LOG_DEBUG("send udp message successful size={}", size);
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
		this->mPort = 0;
		this->mTimer = nullptr;
		this->mUdpSocket = nullptr;
		this->mContext = nullptr;
		this->mKcpServer = nullptr;
	}


	bool KcpComponent::LateAwake()
	{
		return this->StartListen("kcp");
	}

	void KcpComponent::Start()
	{
		std::chrono::milliseconds ms(10);
		this->mContext->post(std::bind(&KcpComponent::Receive, this));
		this->mTimer = std::make_unique<AsioTimer>(*this->mContext, ms);
		this->mTimer->async_wait(std::bind(&KcpComponent::Update, this, std::placeholders::_1));
	}

	void KcpComponent::Update(const asio::error_code& code)
	{
		if(code == asio::error::operation_aborted) //定时器被取消
		{
			return;
		}
		std::chrono::milliseconds ms(10);
		ikcp_update(this->mKcpServer, time(NULL));
		this->mTimer->expires_at(this->mTimer->expiry() + ms);
		this->mContext->post(std::bind(&KcpComponent::Receive, this));
		this->mTimer->async_wait(std::bind(&KcpComponent::Update, this, std::placeholders::_1));
	}

	void KcpComponent::Receive()
	{
		this->mUdpSocket->async_receive_from(asio::buffer(this->mReceiveBuffer), this->mEndpoint, 0,
			[this](const asio::error_code& code, size_t size)
			{
				if (code)
				{
					CONSOLE_LOG_ERROR(code.message());
				}
				else
				{
					unsigned int conv = ikcp_getconv(this->mReceiveBuffer.data());

					int len = 0;
					std::iostream istream(&this->mRecvBuffer);
					istream.write(this->mReceiveBuffer.data(), size);
					std::string ip = this->mEndpoint.address().to_string();
					std::shared_ptr<rpc::Packet> message = std::make_shared<rpc::Packet>();
					std::string address = fmt::format("kcp://{0}:{1}", ip, this->mEndpoint.port());
					//if(message->ParseLen(istream, len) && message->Parse(istream, size -len))
					{
//						asio::io_service & io = this->mApp->MainContext();
//						io.post(std::bind(&KcpComponent::OnMessage, this, message));
					}
					ikcp_input(this->mKcpServer, this->mReceiveBuffer.data(), size);
				}
				this->mContext->post(std::bind(&KcpComponent::Receive, this));
			});
	}

	void KcpComponent::OnMessage(std::shared_ptr<rpc::Packet> message)
	{

	}

	bool KcpComponent::StartListen(const char* name)
	{
		ThreadComponent* threadComponent = this->GetComponent<ThreadComponent>();
		try
		{
			this->mKcpServer = ikcp_create(1, this);
			this->mContext = &threadComponent->GetContext();
			AsioUdpEndpoint endpoint(udp::v4(), this->mPort);
			this->mKcpServer->output = KcpComponent::OnServerSend;
			this->mUdpSocket = std::make_unique<AsioUdpSocket>(*this->mContext, endpoint);
			this->mContext->post([this]{ this->Start();});
			return true;
		}
		catch (const asio::system_error& ec)
		{
			return false;
		}
	}

}


//#include <iostream>
//#include <functional>
//#include <vector>
//#include <chrono>
//#include <asio.hpp>
//#include "ikcp.h"
//
//class udp_server {
//public:
//	udp_server(asio::io_service& io_service, uint16_t port)
//			: socket_(io_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
//	{
//		// 初始化KCP协议对象
//		kcp_ = ikcp_create(0x11223344, this);
//		kcp_->output = output_wrapper;
//		ikcp_wndsize(kcp_, 1024, 1024);
//
//		// 设置KCP协议参数
//		kcp_->rx_minrto = 10;
//		kcp_->rx_maxrto = 100;
//		kcp_->fastresend = 1;
//		kcp_->nocwnd = 1;
//		kcp_->stream = 0;
//		kcp_->dead_link = 5;
//		kcp_->output = output_wrapper;
//
//		// 开启ACK
//		ikcp_nodelay(kcp_, 1, 10, 2, 1);
//
//		start_receive();
//	}
//
//private:
//	void start_receive() {
//		socket_.async_receive_from(asio::buffer(buffer_),
//				endpoint_,
//				std::bind(&udp_server::handle_receive, this, std::placeholders::_1, std::placeholders::_2));
//	}
//
//	void handle_receive(const asio::error_code& error, std::size_t bytes_transferred) {
//		if (!error) {
//			// 将接收到的数据写入到KCP对象中
//			ikcp_input(kcp_, buffer_.data(), bytes_transferred);
//
//			for (int len = ikcp_peeksize(kcp_); len > 0; len = ikcp_peeksize(kcp_)) {
//				std::vector<char> data(len);
//				if (ikcp_recv(kcp_, data.data(), data.size()) < 0) {
//					// 处理接收数据出错
//					continue;
//				}
//
//				// 处理接收到的数据
//				handle_packet(data);
//			}
//
//			start_receive();
//		}
//	}
//
//	void handle_packet(const std::vector<char>& packet) {
//		// 处理接收到的数据
//		std::cout << "Received: " << std::string(packet.begin(), packet.end()) << std::endl;
//
//		// 发送回复消息
//		send_to(endpoint_, "Response");
//	}
//
//	void send_to(const asio::ip::udp::endpoint& endpoint, const std::string& data) {
//		std::vector<char> packet(data.begin(), data.end());
//		ikcp_send(kcp_, packet.data(), packet.size());
//
//		socket_.async_send_to(asio::buffer(packet),
//				endpoint,
//				std::bind(&udp_server::handle_send, this, std::placeholders::_1, std::placeholders::_2));
//	}
//
//	void handle_send(const asio::error_code& /*error*/, std::size_t /*bytes_transferred*/) {
//	}
//
//	static int output_wrapper(const char *buf, int len, ikcpcb */*kcp*/, void *user) {
//		udp_server *server = static_cast<udp_server*>(user);
//		std::vector<char> packet(buf, buf + len);
//		server->socket_.async_send_to(asio::buffer(packet),
//				server->endpoint_,
//				std::bind(&udp_server::handle_send, server, std::placeholders::_1, std::placeholders::_2));
//		return 0;
//	}
//
//private:
//	std::array<char, 1024> buffer_;
//	asio::ip::udp::endpoint endpoint_;
//	asio::ip::udp::socket socket_;
//	IKCPCB* kcp_;
//};
//
//int main(int argc, char *argv[]) {
//	try {
//		asio::io_service io_service;
//		udp_server server(io_service, 8000);
//
//		io_service.run();
//	} catch (std::exception& e) {
//		std::cerr << "Exception: " << e.what() << std::endl;
//	}
//
//	return 0;
//}
//```
//
//在示例代码中，我们创建了一个名为udp_server的类，它包含一个UDP socket和一个KCP对象kcp_来实现可靠UDP通信。在构造函数中，我们创建了UDP socket并将其绑定到指定的端口上，并初始化了KCP对象kcp_。
//
//在start_receive函数中，我们使用socket_.async_receive_from函数开启异步UDP数据包接收，并在回调函数handle_receive中将接收到的数据写入kcp_中，并调用ikcp_recv函数获取已经接收到的可靠数据包，在handle_packet函数中进行进一步处理。
//
//在发送数据包时，我们使用ikcp_send函数将数据包写入kcp_中，并使用socket_.async_send_to函数将数据包发送给对端，同时在回调函数handle_send中进行处理。
//
//最后，我们启动Asio的IO Service，循环调用io_service.run函数，同时在循环中调用ikcp_update函数实现KCP协议的更新