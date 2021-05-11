#include "NetWorkAction.h"

namespace SoEasy
{
	XCode LocalActionProxy1::Invoke(shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		const long long operId = requestData->operator_id();
		return this->mBindAction(operId);
	}
}
