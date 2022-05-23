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
#ifndef ONLY_MAIN_THREAD
		int networkCount = 1;
		LOG_CHECK_RET(config.GetMember("thread", "network", networkCount));
		for (int index = 0; index < networkCount; index++)
		{
			this->mNetThreads.push_back(new NetWorkThread());
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
