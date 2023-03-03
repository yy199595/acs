#include"NetThreadComponent.h"
#include"App/App.h"
#include"Method/MethodProxy.h"
#include"Tcp/SocketProxy.h"

namespace Sentry
{
    bool AsioThread::Run()
    {
        if(this->mContext || this->mThread)
        {
            return false;
        }
        this->mContext = std::make_unique<Asio::Context>();
        this->mThread = std::make_unique<std::thread>(std::bind(&AsioThread::Update, this));
        this->mThread->detach();
        return true;
    }

    void AsioThread::Update()
    {
        asio::error_code code;
        Asio::ContextWork work(*this->mContext);
        while (!this->mContext->stopped())
        {
            this->mContext->run(code);
        }
    }
}

namespace Sentry
{
	bool NetThreadComponent::Awake()
	{
		this->mIndex = 0;
		int taskCount = 0;
		const ServerConfig * config = ServerConfig::Inst();
		config->GetMember("thread", "task", taskCount);
#ifdef __ENABLE_OPEN_SSL__
		std::string sslFile;
		config.GetPath("ssl", sslFile);
		IAsioThread & t = this->GetApp()->GetTaskScheduler();
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
            std::unique_ptr<AsioThread> asioThread
                = std::make_unique<AsioThread>();
            {
                this->mNetThreads.emplace_back(std::move(asioThread));
            }
#ifdef __ENABLE_OPEN_SSL__
			this->mNetThreads[index]->LoadVeriftFile(sslFile);
#endif
		}
#endif
        return true;
	}

	bool NetThreadComponent::LateAwake()
    {
#ifndef ONLY_MAIN_THREAD
        for (size_t index = 0; index < this->mNetThreads.size(); index++)
        {
            if(!this->mNetThreads[index]->Run())
            {
                return false;
            }
           // CONSOLE_LOG_INFO("start net thread [" << index << "] successful");
        }
#endif
        return true;
    }

	void NetThreadComponent::OnDestroy()
	{

	}

    std::shared_ptr<SocketProxy> NetThreadComponent::CreateSocket()
    {
#ifdef ONLY_MAIN_THREAD
        asio::io_service & io = this->mApp->MainThread();
#else
        if (this->mIndex >= mNetThreads.size())
        {
            this->mIndex = 0;
        }
        asio::io_service &io = mNetThreads[this->mIndex++]->Context();
#endif
        return std::make_shared<SocketProxy>(io);
    }

    std::shared_ptr<SocketProxy> NetThreadComponent::CreateSocket(const std::string &ip, unsigned short port)
    {
        std::shared_ptr<SocketProxy> socket = this->CreateSocket();
        socket->Init(ip, port);
        return socket;
    }
	std::shared_ptr<SocketProxy> NetThreadComponent::CreateSocket(const string& address)
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
