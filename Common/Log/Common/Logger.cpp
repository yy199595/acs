//
// Created by yy on 2023/8/12.
//

#include "Logger.h"
#include"Core/Thread/ThreadSync.h"
namespace custom
{
	Logger::Logger(std::string name, Asio::Context& io)
		: mName(std::move(name)), mContext(io), mTimer(io)
	{
		this->mTick = 0;
		this->mSaveTime = 5;
	}

	void Logger::Close()
	{
#ifdef ONLY_MAIN_THREAD
		for(IOutput * output : this->mOutputs)
		{
			output->Close();
		}
#else
		custom::ThreadSync<bool> threadSync;
		this->mContext.post([this, &threadSync]
		{
			for(IOutput * output : this->mOutputs)
			{
				output->Close();
			}
			threadSync.SetResult(true);
		});
		threadSync.Wait();
#endif

	}

	void Logger::Flush()
	{

#ifdef ONLY_MAIN_THREAD
		for(IOutput * output : this->mOutputs)
		{
			output->Flush();
		}
#else
		this->mContext.post([this]
		{
			for(IOutput * output : this->mOutputs)
			{
				output->Flush();
			}
		});
#endif

	}

	void Logger::SetLevel(custom::LogLevel level)
	{
#ifdef ONLY_MAIN_THREAD
		for(IOutput * output : this->mOutputs)
		{
			output->SetLevel(level);
		}
#else
		this->mContext.post([this, level]
			{
				for(IOutput * output : this->mOutputs)
				{
					output->SetLevel(level);
				}
			});
#endif

	}

	bool Logger::Start()
	{
#ifdef ONLY_MAIN_THREAD
		for(IOutput * output : this->mOutputs)
		{
			if(!output->Start(this->mContext))
			{
				return false;
			}
		}
		return true;
#else
		custom::ThreadSync<bool> threadSync;
		this->mContext.post([this, &threadSync]
		{
			for(IOutput * output : this->mOutputs)
			{
				if(!output->Start(this->mContext))
				{
					threadSync.SetResult(false);
					return;
				}
			}
			threadSync.SetResult(true);
			this->mTimer.expires_after(std::chrono::seconds(this->mSaveTime));
			this->mTimer.async_wait([this](auto && PH1) { OnTimer(std::forward<decltype(PH1)>(PH1)); });
		});
		return threadSync.Wait();
#endif

	}

	void Logger::OnTimer(const Asio::Code& code)
	{
		if(!code)
		{
			this->mTick++;
			for(IOutput * output : this->mOutputs)
			{
				output->OnTick(this->mTick);
			}
		}
		this->mTimer.expires_after(std::chrono::seconds(this->mSaveTime));
		this->mTimer.async_wait([this](auto && PH1) { OnTimer(std::forward<decltype(PH1)>(PH1)); });
	}

	void Logger::Push(std::unique_ptr<LogInfo> log)
	{
#ifdef ONLY_MAIN_THREAD
		for(IOutput * output : this->mOutputs)
		{
			output->Push(this->mContext, this->mName, *log);
		}
#else
		this->mContext.post([this, logInfo = log.release()]
		{
			for(IOutput * output : this->mOutputs)
			{
				output->Push(this->mContext, this->mName, *logInfo);
			}
			delete logInfo;
		});
#endif

	}
}
