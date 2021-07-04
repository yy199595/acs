#pragma once
#include<list>
#include<queue>
#include<thread>
#include<memory>
#include"Manager.h"
#include<condition_variable>
#include<Timer/TimerBase.h>
#define TimerPrecision 20 //精度  毫秒
#define MaxMinute 1		//一圈轮询时间 分钟
namespace SoEasy
{
	class TimerManager : public Manager
	{
	public:
		TimerManager();
		~TimerManager() { }
	public:
		void Stop();
		bool RemoveTimer(long long id);
		bool AddTimer(shared_ptr<TimerBase> timer);
		bool AddTimer(long long ms, std::function<void(void)> func);
		shared_ptr<TimerBase> GetTimer(long long id);
	public:
		template<typename T, typename ... Args>
		shared_ptr<T> CreateTimer(Args && ...args);
 	protected:
		bool OnInit() { return true; }
		void OnInitComplete() final;
		void OnSystemUpdate() final;					//处理系统事件
	private:
		void RefreshTimer();
		bool AddNextTimer(shared_ptr<TimerBase> timer);
		void AddTimerToWheel(shared_ptr<TimerBase> timer);
	private:
		bool mIsStop;
		std::thread * mTimerThread;
		std::mutex mTimerThreadLock;
		std::condition_variable mWheelVariable;		//是否轮询的条件变量		
		DoubleBufferQueue<long long> mFinishTimerQueue;	//完成一次的队列
		DoubleBufferQueue<shared_ptr<TimerBase>> mWheelQueue;	//需要轮询的队列
		std::unordered_map<long long, shared_ptr<TimerBase>> mTimerMap;	//所有timer的列表
	private:
		int mTickCount;
		int mTimerCount;
		int mMaxTimeCount;
		long long mStartTime;
		std::list<shared_ptr<TimerBase>> mNextWheelTimer;	// 本次不关注的定时器
		std::queue<shared_ptr<TimerBase>> mTimers[MaxMinute * 60 * (1000 / TimerPrecision)];
	};
	template<typename T, typename ...Args>
	inline shared_ptr<T> TimerManager::CreateTimer(Args && ...args)
	{
		std::shared_ptr<T> timer = std::make_shared<T>(std::forward<Args>(args)...);
		return this->AddTimer(timer) ? timer : nullptr;
		
	}

}