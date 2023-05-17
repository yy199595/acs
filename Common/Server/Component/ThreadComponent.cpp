#include"ThreadComponent.h"
#include"Entity/Actor/App.h"
#include"Rpc/Method/MethodProxy.h"
#include"Network/Tcp/SocketProxy.h"
#ifdef __ENABLE_OPEN_SSL__
#include<asio/ssl.hpp>
#endif
using namespace asio::ip;
namespace Tendo
{
    AsioThread::AsioThread()
        : mThread(nullptr),
         mContext(nullptr)
    {

    }

    void AsioThread::Run()
    {
        this->mContext = new Asio::Context();
        std::shared_ptr<Asio::ContextWork> work
            = std::make_shared<Asio::ContextWork>(*this->mContext);
        this->mThread = new std::thread([this, work]()
            {
                asio::error_code code;
                this->mContext->run(code);
            });
        this->mThread->detach();
    }
}

namespace Tendo
{
    bool ThreadComponent::Awake()
	{
		int taskCount = 0;
		const ServerConfig* config = ServerConfig::Inst();
		config->GetMember("thread", "task", taskCount);
#ifdef __ENABLE_OPEN_SSL__
		// Create an SSL context
		asio::ssl::context sslContext(asio::ssl::context::sslv23);

// Load the certificate file
		sslContext.load_verify_file("path/to/certificate.pem");

// Create an SSL socket using the SSL context
		std::shared_ptr<SocketProxy> socket = this->CreateSocket();
		asio::ssl::stream<asio::ip::tcp::socket&> sslSocket(socket->GetSocket(), sslContext);

// Connect to the HTTPS endpoint of Baidu Calendar
		asio::ip::tcp::resolver resolver(socket->GetThread());
		asio::ip::tcp::resolver::query query("calendar.baidu.com", "https");
		asio::ip::tcp::resolver::iterator endpointIterator = resolver.resolve(query);

// Perform SSL handshake
		asio::connect(sslSocket.lowest_layer(), endpointIterator);
		sslSocket.handshake(asio::ssl::stream_base::client);

// Send an HTTPS request to Baidu Calendar to search for "日历"
		std::string request = "GET /s?wd=%E6%97%A5%E5%8E%86 HTTP/1.1\r\n"
							  "Host: calendar.baidu.com\r\n"
							  "Connection: close\r\n\r\n";
		asio::write(sslSocket, asio::buffer(request));
#endif
#ifndef ONLY_MAIN_THREAD
		int networkCount = 1;
		std::lock_guard<std::mutex> lock(this->mMutex);
		config->GetMember("thread", "network", networkCount);
		for (int index = 0; index < networkCount; index++)
		{
			this->mNetThreads.push_back(new AsioThread());
		}

		for (AsioThread* thread : this->mNetThreads)
		{
			thread->Run();
		}
#endif
		return true;
	}


	void ThreadComponent::OnDestroy()
	{
#ifndef ONLY_MAIN_THREAD
        for (AsioThread* thread : this->mNetThreads)
        {
            delete thread;
        }
        this->mNetThreads.clear();
		CONSOLE_LOG_INFO("delete net work thread");
#endif
	}

    Asio::Context& ThreadComponent::GetContext()
	{
#ifdef ONLY_MAIN_THREAD
		return this->mApp->MainThread();
#else
		std::lock_guard<std::mutex> lock(this->mMutex);
		AsioThread* t = this->mNetThreads.front();
		{
			this->mNetThreads.pop_front();
			this->mNetThreads.push_back(t);
		}
		return t->Context();
#endif
	}

    std::shared_ptr<Tcp::SocketProxy> ThreadComponent::CreateSocket(const std::string & net)
    {
        std::shared_ptr<Tcp::SocketProxy> socket;
#ifdef ONLY_MAIN_THREAD
        asio::io_service & io = this->mApp->MainThread();
        socket = std::make_shared<Tcp::SocketProxy>(io);
#else
		std::lock_guard<std::mutex> lock(this->mMutex);
		if(this->mNetThreads.empty())
		{
			asio::io_service & io = this->mApp->MainThread();
			return std::make_shared<Tcp::SocketProxy>(io, net);
		}
        AsioThread* t = this->mNetThreads.front();
        {
            socket = std::make_shared<Tcp::SocketProxy>(t->Context(), net);
        }
        this->mNetThreads.pop_front();
        this->mNetThreads.push_back(t);
#endif
        return socket;
    }

    std::shared_ptr<Tcp::SocketProxy> ThreadComponent::CreateSocket(const std::string & net, const std::string &ip, unsigned short port)
    {
        std::shared_ptr<Tcp::SocketProxy> socket = this->CreateSocket(net);
        {
            socket->Init(ip, port);
        }
        return socket;
    }
	std::shared_ptr<Tcp::SocketProxy> ThreadComponent::CreateSocket(const std::string & net, const std::string& address)
	{
		size_t pos = address.find(':');
		if(pos == std::string::npos)
		{
			return nullptr;
		}
		const std::string ip = address.substr(0, pos);
		const std::string port = address.substr(pos + 1);
		return this->CreateSocket(net, ip, (unsigned short)std::stoi(port));
	}
}
