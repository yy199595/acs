//
// Created by leyi on 2023/8/8.
//

#include "AsioThread.h"
#include "Util/Time/TimeHelper.h"
namespace custom
{
	AsioThread::AsioThread(int update)
		:mContext(1), mTime(update)
	{

	}

	void AsioThread::Stop()
	{
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
		//std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	void AsioThread::Run()
	{
		asio::error_code code;
		Asio::ContextWork work(this->mContext);
		{
			while(!this->mContext.stopped())
			{
				this->mContext.run(code);
			}
		}
//		while(!this->mContext.stopped())
//		{
//			this->mLastTime = help::Time::NowSec();
//			this->mContext.run_one_for(std::chrono::seconds(5));
//			//printf("[%s:%d] invoke once event\n", this->mName.c_str(), this->mId);
//		}
	}
}