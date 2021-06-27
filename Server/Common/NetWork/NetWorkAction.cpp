#include "NetWorkAction.h"

namespace SoEasy
{
	XCode LocalActionProxy1::Invoke(shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		const long long operId = requestData->entityid();
		return this->mBindAction(operId);
	}
}
