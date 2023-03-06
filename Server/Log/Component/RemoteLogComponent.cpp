#include"RemoteLogComponent.h"
#include"Service/Log.h"
namespace Sentry
{
    bool RemoteLogComponent::LateAwake()
    {
        this->mRpcService = this->mApp->GetService<Log>();
        return this->mRpcService != nullptr;
    }
}
