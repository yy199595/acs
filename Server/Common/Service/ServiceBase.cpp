#include"ServiceBase.h"

namespace SoEasy
{
	XCode ServiceBase::CallAction(SharedPacket request, SharedPacket returnData)
	{
		const std::string & action = request->action();
		auto iter = this->mActionMap.find(action);
		if (iter == this->mActionMap.end())
		{
			SayNoDebugError("call func not find " << this->GetTypeName()
				<< "." << action);
			return XCode::CallFunctionNotExist;
		}
		shared_ptr<LocalActionProxy> actionProxy = iter->second;
		return actionProxy->Invoke(request, returnData);
	}

	bool ServiceBase::BindFunction(std::string name, LocalAction1 action)
	{
		return this->BindFunction(name, make_shared<LocalActionProxy1>(action, name));
	}
	bool ServiceBase::BindFunction(const std::string & name, shared_ptr<LocalActionProxy> actionBox)
	{
		auto iter = this->mActionMap.find(name);
		if (iter != this->mActionMap.end())
		{
			SayNoDebugError("register " << this->GetTypeName() << "." << name << " fail");
			return false;
		}
		this->mActionMap.emplace(name, actionBox);
		return true;
	}
}