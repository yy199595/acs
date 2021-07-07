#include"NetWorkAction.h"
#include<Pool/ProtocolPool.h>
namespace SoEasy
{
	XCode LocalActionProxy1::Invoke(PB::NetWorkPacket * messageData)
	{
		const long long operId = messageData->entityid();
#ifdef _DEBUG
		SayNoDebugWarning("[request ] [" << this->mServiceName << "." << this->GetName() << "]");
#endif
		XCode code = this->mBindAction(operId);
#ifdef _DEBUG
		if (messageData->rpcid() != 0)
		{
			SayNoDebugWarning("[response] [" << this->mServiceName << "." << this->GetName() << "]");
		}
#endif
		return code;
	}
}