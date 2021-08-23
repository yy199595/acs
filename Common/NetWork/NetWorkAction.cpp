#include"NetWorkAction.h"

namespace Sentry
{
    bool LocalActionProxy1::Invoke(NetMessageProxy *messageData)
    {
        long long userId = messageData->GetUserId();
		messageData->ClearMessage();
		messageData->SetCode(this->mBindAction(userId));
		return true;
    }
}