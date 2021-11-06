#include "SocketProxy.h"
#include<Util/NumberHelper.h>
namespace GameKeeper
{
	StringPool GStringPool;
	SocketProxy::SocketProxy(NetWorkThread & thread, const std::string & name)
		: mNetThread(thread), mContext(thread.GetContext()),
		mSocket(thread.GetContext()), mName(name)
	{
		this->mSocketId = NumberHelper::Create();
	}
	SocketProxy::~SocketProxy()
	{
		if (this->IsOpen())
		{
			asio::error_code err;
			this->mSocket.shutdown(asio::socket_base::shutdown_both, err);
			this->mSocket.close(err);
		}
	}
	bool SocketProxy::IsOpen()
	{
		std::lock_guard<std::mutex> lock(this->mLock);
		return this->mSocket.is_open();
	}
}
