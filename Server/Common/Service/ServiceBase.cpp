#include"ServiceBase.h"
#include<NetWork/NetWorkRetAction.h>
namespace Sentry
{
	ServiceBase::ServiceBase()
	{
		this->mIsInit = false;
	}

	void ServiceBase::InitService(const std::string & serviceName)
	{
		if (this->mIsInit == false)
		{
			this->mIsInit = true;
			this->mServiceName = serviceName;
			this->Init(Applocation::Get(), serviceName);
		}
	}
}
