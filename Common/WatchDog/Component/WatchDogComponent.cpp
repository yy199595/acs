#include"WatchDogComponent.h"
#include"Config/ServerConfig.h"
#include"Service/WatchDog.h"
#include"Component/NodeMgrComponent.h"
#include"Component/InnerNetComponent.h"
namespace Sentry
{
	bool WatchDogComponent::LateAwake()
	{
		this->mAddress = fmt::format("127.0.0.1:{0}", 3344);
		return true;
	}

	void WatchDogComponent::OnSecondUpdate(int tick)
	{
		if(tick % 10 == 0)
		{
			RpcService * rpcService = this->mApp->GetService<WatchDog>();
			if(rpcService != nullptr)
			{
				rpcService->Send(this->mAddress, "Ping");
			}
		}
	}
}