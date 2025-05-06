//
// Created by leyi on 2023/8/8.
//

#include "AsioThread.h"
#include "Util/Tools/TimeHelper.h"
namespace custom
{
	AsioThread::AsioThread(int update)
		:mContext(1), mUpdate(update)
	{
		this->mId = 0;
		this->mCount = 0;
		this->mLastTime = 0;
	}

	void AsioThread::Stop()
	{
		Asio::Code code;
		this->mContext.stop();
		if(this->mThread.joinable())
		{
			this->mThread.join();
		}
	}

	void AsioThread::Start(int id, const std::string & name)
	{
		this->mId = id;
		this->mName = name;
		std::thread(&AsioThread::Run, this).swap(this->mThread);
	}

	void AsioThread::Run()
	{
		std::chrono::seconds sleep(this->mUpdate);
		auto work = asio::make_work_guard(this->mContext);
		while (!this->mContext.stopped())
		{
			this->mCount += this->mContext.run_one_for(sleep);
			this->mLastTime = help::Time::NowSec();
		}
//		while(!this->mContext.stopped())
//		{
//			this->mLastTime = help::Time::NowSec();
//			this->mContext.run_one_for(std::chrono::seconds(5));
//			//printf("[%s:%d] invoke once event\n", this->mName.c_str(), this->mId);
//		}
	}
}