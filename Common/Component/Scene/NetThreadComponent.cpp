#include "NetThreadComponent.h"
#include "Util/Guid.h"
#include "App/App.h"
#include "Method/MethodProxy.h"
#include"Network/SocketProxy.h"
namespace Sentry
{
	void NetThreadComponent::Awake()
	{
		this->mIndex = 0;
		int taskCount = 0;
		const ServerConfig& config = App::Get()->GetConfig();
		config.GetMember("thread", "task", taskCount);
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
		LOG_CHECK_RET(config.GetMember("thread", "network", networkCount));
		for (int index = 0; index < networkCount; index++)
		{
            asio::io_service * io = new asio::io_service();
            asio::io_service::work * work = new asio::io_service::work(*io);
            this->mNetThreads.emplace_back(io);
            this->mNetThreadWorks.emplace_back(work);
#ifdef __ENABLE_OPEN_SSL__
			this->mNetThreads[index]->LoadVeriftFile(sslFile);
#endif
		}
#endif
	}

	bool NetThreadComponent::LateAwake()
	{
#ifndef ONLY_MAIN_THREAD
		for (asio::io_service * io: this->mNetThreads)
        {
            std::thread *t = new std::thread([io](){
                asio::error_code code;
                io->run(code);
            });
            t->detach();
            this->mThreads.emplace_back(t);
        }
#endif
		return true;
	}

	void NetThreadComponent::OnDestory()
	{

	}

    std::shared_ptr<SocketProxy> NetThreadComponent::CreateSocket()
    {
#ifdef ONLY_MAIN_THREAD
            asio::io_service & io = this->GetApp()->GetThread();
#else
            if (this->mIndex >= mNetThreads.size())
            {
                this->mIndex = 0;
            }
            asio::io_service &io = *(mNetThreads[this->mIndex++]);
#endif
        return std::make_shared<SocketProxy>(io);
    }

    std::shared_ptr<SocketProxy> NetThreadComponent::CreateSocket(const std::string &ip, unsigned short port)
    {
        std::shared_ptr<SocketProxy> socket = this->CreateSocket();
        socket->Init(ip, port);
        return socket;
    }
}
