//
// Created by 64658 on 2024/9/21.
//

#include "MongoProxyComponent.h"

namespace acs
{
	bool MongoProxyComponent::LateAwake()
	{
		this->mMongo = this->GetComponent<MongoComponent>();
		return true;
	}
}