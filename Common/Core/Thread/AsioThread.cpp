//
// Created by leyi on 2023/8/8.
//

#include "AsioThread.h"
#include "Util/Tools/TimeHelper.h"
namespace custom
{
	AsioThread::AsioThread(int update)
		:mContext(1), mUpdate(update), mTimer(mContext)
	{
		this->mId = 0;
		this->mLastTime = 0;
	}

	void AsioThread::Stop()
	{
		Asio::Code code;
		this->mContext.stop();
		if(this->mThread.joinable())
		{
			this->mThread.join();
			this->mTimer.cancel(code);
		}
	}

	void AsioThread::Start(int id, const std::string & name)
	{
		this->mId = id;
		this->mName = name;
		std::thread(&AsioThread::Run, this).swap(this->mThread);
	}

	void AsioThread::StartTimer()
	{
		this->mLastTime = help::Time::NowSec();
		this->mTimer.expires_after(std::chrono::seconds(this->mUpdate));
		this->mTimer.async_wait([self = this->shared_from_this()](const asio::error_code& code)
		{
			self->StartTimer();
			self->mLastTime = help::Time::NowSec();
		});
	}

	void AsioThread::Run()
	{
		asio::error_code code;
		Asio::ContextWork work(this->mContext);
		while (!this->mContext.stopped())
		{
			this->StartTimer();
			this->mContext.run(code);
		}
//		while(!this->mContext.stopped())
//		{
//			this->mLastTime = help::Time::NowSec();
//			this->mContext.run_one_for(std::chrono::seconds(5));
//			//printf("[%s:%d] invoke once event\n", this->mName.c_str(), this->mId);
//		}
	}
}