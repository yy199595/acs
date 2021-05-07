#include "NetWorkAction.h"

namespace SoEasy
{
	XCode NetWorkActionBox1::Invoke(shared_ptr<TcpClientSession> session, const shared_ptr<NetWorkPacket> requestData, shared_ptr<NetWorkPacket> returnData)
	{
		const long long operId = requestData->operator_id();
		return this->mLuaFunction == nullptr
			? this->mBindAction(session, operId)
			: this->mLuaFunction->Invoke1(session, operId);
	}
	void NetWorkActionBox::BindLuaFunction(NetLuaAction * func)
	{
		if (this->mLuaFunction != nullptr)
		{
			delete this->mLuaFunction;
			this->mLuaFunction = nullptr;
		}
		this->mLuaFunction = func;
	}
}
