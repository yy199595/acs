#pragma once
#include<list>
#include<thread>
#include<memory>
#include"Manager.h"
#include<condition_variable>
#include<Timer/TimerBase.h>
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
	private:
		bool mIsStop;
		long long mWheelInterval;		//轮询间隔
		std::thread * mTimerThread;
		std::mutex mTimerThreadLock;
		std::condition_variable mWheelVariable;		//是否轮询的条件变量
		std::queue<shared_ptr<TimerBase>> mWheelQueue;	//正在轮询队列
		std::queue<shared_ptr<TimerBase>> mNextWheelQueue;	//下次轮询队列
		DoubleBufferQueue<long long> mFinishTimerQueue;	//完成一次的队列
		std::unordered_map<long long, shared_ptr<TimerBase>> mTimerMap;	//所有timer的列表
	};
	template<typename T, typename ...Args>
	inline shared_ptr<T> TimerManager::CreateTimer(Args && ...args)
	{
		std::shared_ptr<T> timer = std::make_shared<T>(std::forward<Args>(args)...);
		return this->AddTimer(timer) ? timer : nullptr;
		
	}

}