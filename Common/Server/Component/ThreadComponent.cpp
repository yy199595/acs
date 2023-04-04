#include"ThreadComponent.h"
#include"Entity/App/App.h"
#include"Rpc/Method/MethodProxy.h"
#include"Network/Tcp/SocketProxy.h"

namespace Sentry
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

namespace Sentry
{
    bool ThreadComponent::Awake()
    {
        int taskCount = 0;
        const ServerConfig* config = ServerConfig::Inst();
        config->GetMember("thread", "task", taskCount);
#ifdef __ENABLE_OPEN_SSL__
        std::string sslFile;
        config.GetPath("ssl", sslFile);
        IAsioThread& t = this->GetApp()->GetTaskScheduler();
        this->GetApp()->GetTaskScheduler().LoadVeriftFile("");

        asio::ssl::stream<asio::ip::tcp::socket> sslSocket(t, t.GetSSL());

        tcp::resolver resolver(t);
        tcp::resolver::query query("https://baidu.com", "https");
        asio::connect(sslSocket.lowest_layer(), resolver.resolve(query));
        sslSocket.lowest_layer().set_option(tcp::no_delay(true));

        // 执行SSL握手并认证远程主机的证书
        sslSocket.set_verify_mode(asio::ssl::verify_peer);
        sslSocket.set_verify_callback(asio::ssl::host_name_verification("https://baidu.com"));
        sslSocket.handshake(asio::ssl::stream_base::client);

#endif
#ifndef ONLY_MAIN_THREAD
        int networkCount = 1;
        config->GetMember("thread", "network", networkCount);
        for (int index = 0; index < networkCount; index++)
        {
            this->mNetThreads.push_back(new AsioThread());
#ifdef __ENABLE_OPEN_SSL__
            this->mNetThreads[index]->LoadVeriftFile(sslFile);
#endif
        }
#endif
        return true;
    }

	bool ThreadComponent::LateAwake()
    {
#ifndef ONLY_MAIN_THREAD
        for (AsioThread* thread : this->mNetThreads)
        {
            thread->Run();
        }
#endif
        return true;
    }

	void ThreadComponent::OnDestroy()
	{
        for (AsioThread* thread : this->mNetThreads)
        {
            delete thread;
        }
        this->mNetThreads.clear();
		CONSOLE_LOG_INFO("delete net work thread");
	}

    Asio::Context& ThreadComponent::GetContext()
    {
#ifdef ONLY_MAIN_THREAD
        return this->mApp->MainThread();
#else
        AsioThread* t = this->mNetThreads.front();
        {
            this->mNetThreads.pop_front();
            this->mNetThreads.push_back(t);
        }
#endif
        return t->Context();
    }

    std::shared_ptr<SocketProxy> ThreadComponent::CreateSocket()
    {
        std::shared_ptr<SocketProxy> socket;
#ifdef ONLY_MAIN_THREAD
        asio::io_service & io = this->mApp->MainThread();
        socket = std::make_shared<SocketProxy>(io);
#else
		if(this->mNetThreads.empty())
		{
			asio::io_service & io = this->mApp->MainThread();
			return std::make_shared<SocketProxy>(io);
		}
        AsioThread* t = this->mNetThreads.front();
        {
            socket = std::make_shared<SocketProxy>(t->Context());
        }
        this->mNetThreads.pop_front();
        this->mNetThreads.push_back(t);
#endif
        return socket;
    }

    std::shared_ptr<SocketProxy> ThreadComponent::CreateSocket(const std::string &ip, unsigned short port)
    {
        std::shared_ptr<SocketProxy> socket = this->CreateSocket();
        {
            socket->Init(ip, port);
        }
        return socket;
    }
	std::shared_ptr<SocketProxy> ThreadComponent::CreateSocket(const std::string& address)
	{
		size_t pos = address.find(':');
		if(pos == std::string::npos)
		{
			return nullptr;
		}
		const std::string ip = address.substr(0, pos);
		const std::string port = address.substr(pos + 1);
		return this->CreateSocket(ip, (unsigned short)std::stoi(port));
	}
}
