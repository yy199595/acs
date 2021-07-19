#include"NetWorkAction.h"
#include<Pool/ProtocolPool.h>
namespace Sentry
{
	XCode LocalActionProxy1::Invoke(long long id, Message * request, Message * response)
	{
		return this->mBindAction(id);
	}
}