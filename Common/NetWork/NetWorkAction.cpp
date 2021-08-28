#include"NetWorkAction.h"

namespace Sentry
{
    XCode LocalActionProxy1::Invoke(NetMessageProxy *messageData)
    {
        long long userId = messageData->GetUserId();
		messageData->ClearMessage();	
		return this->mBindAction(userId);
    }
}