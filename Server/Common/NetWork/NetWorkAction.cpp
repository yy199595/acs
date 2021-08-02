#include"NetWorkAction.h"
namespace Sentry
{
	bool LocalActionProxy1::Invoke(NetMessageProxy * messageData)
	{
		long long userId = messageData->GetUserId();
		XCode code = this->mBindAction(userId);
		return messageData->InitMessageData(code, nullptr);
	}
}