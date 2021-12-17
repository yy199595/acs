#include"SocketProxy.h"
#include<Util/Guid.h>
namespace GameKeeper
{
	SocketProxy::SocketProxy(NetWorkThread & thread, const std::string & name)
		: mNetThread(thread), mContext(thread.GetContext()),
		mSocket(thread.GetContext()), mName(name)
	{
		this->mSocketId = Helper::ThreadGuid::Create();
	}
	
	void SocketProxy::Close()
	{
		if (this->IsOpen())
		{
			asio::error_code code;
			this->mSocket.close(code);
		}	
	}

	bool SocketProxy::IsOpen()
	{
		std::lock_guard<std::mutex> lock(this->mLock);
		return this->mSocket.is_open();
	}
}
