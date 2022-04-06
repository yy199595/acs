#include"RpcServiceBase.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include<Component/Rpc/RpcConfigComponent.h>
#ifdef __DEBUG__
#include"Pool/MessagePool.h"
#endif
#include"Util/TimeHelper.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/Rpc/RpcConfigComponent.h"

namespace Sentry
{
	ServiceRecord::ServiceRecord()
	{
		this->mCallCount = 0;
		this->mStartTime = 0;
	}

	void ServiceRecord::OnCall(long long ms)
	{
		this->mCallCount++;
		this->mStartTime += ms;
	}
	long long ServiceRecord::GetWeight()
	{
		return this->mStartTime / this->mCallCount;
	}
}

namespace Sentry
{
	bool RpcServiceBase::LateAwake()
	{
		this->mRpcClientComponent = this->GetComponent<RpcClientComponent>();
		this->mRpcConfigComponent = this->GetComponent<RpcConfigComponent>();
		return true;
	}
}
