#include "RpcTaskSource.h"
#include"Component/Rpc/RpcHandlerComponent.h"
#ifdef __DEBUG__
#include"Global/ServiceConfig.h"
#endif

namespace Sentry
{
    void RpcTaskSource::OnResponse(std::shared_ptr<com::Rpc_Response> response)
    {
        this->mTaskSource.SetResult(response);
    }

	std::shared_ptr<com::Rpc_Response> RpcTaskSource::Await()
	{
		return this->mTaskSource.Await();
	}
}
