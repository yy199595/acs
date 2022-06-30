#include "NetThreadComponent.h"
#include "Util/Guid.h"
#include "App/App.h"
#include "Method/MethodProxy.h"
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
		IAsioThread & t = this->GetApp()->GetTaskScheduler();

		LOG_CHECK_RET(config.GetMember("thread", "network", networkCount));
		for (int index = 0; index < networkCount; index++)
		{
			this->mNetThreads.push_back(new NetWorkThread());
#ifdef __ENABLE_OPEN_SSL__
			this->mNetThreads[index]->LoadVeriftFile(sslFile);
#endif
		}
#endif
	}

	bool NetThreadComponent::LateAwake()
	{
#ifndef ONLY_MAIN_THREAD
		for (auto taskThread: this->mNetThreads)
		{
			taskThread->Start();
		}
#endif
		return true;
	}

	void NetThreadComponent::OnDestory()
	{

	}

#ifndef ONLY_MAIN_THREAD
	IAsioThread & NetThreadComponent::AllocateNetThread()
	{
		std::lock_guard<std::mutex> lock(this->mLock);
		if (this->mIndex >= mNetThreads.size())
		{
			this->mIndex = 0;
		}
		return *(mNetThreads[this->mIndex++]);
	}
#endif
}
