#pragma once

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif
#include<thread>
#include<CommonDefine/CommonDef.h>
#include<CommonGlobal/ServerConfig.h>
#include<CommonDefine/ClassStatement.h>
#include<CommonOther/TimeRecorder.h>
using namespace std;
using namespace asio::ip;

namespace SoEasy
{
	class Manager;
	class Applocation
	{
	public:
		Applocation(const std::string srvName, const std::string configDirectory = "./Config/");
		virtual ~Applocation() {};
	public:
		inline float GetFps() { return this->mFps; }
		inline float GetDelaTime() { return this->mDelatime; }
		inline long long GetLogicTime() { return this->mLogicTime; }
		inline long long GetStartTime() { return this->mStartTime; }
		inline AsioContext & GetContext() { return this->mAsioContext; }
		inline ServerConfig & GetServerConfig() { return this->mConfig; }
		inline const std::string & GetServerName() { return this->mConfig.GetServerName(); }
		inline const std::string & GetConfigDirectory() { return this->mSrvConfigDirectory; }
	public:
		static Applocation * Get() { return mApplocation; }
	public:
		template<typename T> 
		shared_ptr<T> AddManager();
		template<typename T>
		shared_ptr<T> GetManager();
		template<typename T>
		shared_ptr<T> GetOrAddManager();
		shared_ptr<Manager> GetManagerByName(const std::string name);
		std::vector<shared_ptr<Manager>> & GetAllManager() { return this->mSortManagerArray; }
	public:
		TimeRecorder & GetRecoder() { return this->mMainLogicTimeRecorder; }
	public:
		int Run();
		int Stop();
		float GetMeanFps();
	private:
		bool InitMember();
		bool InitManager();
		int	 OnExit(int code);
		void UpdateConsoleTitle();
	private:
		void OnSecondUpdate();
		void OnFrameUpdate(float t);
		void OnFrameUpdateAfter();
	private:
		int  LogicMainLoop();
	private:
		float mFps;
		bool mIsClose;
		float mDelatime;
		ServerConfig mConfig;
		long long mLogicTime;
		long long mStartTime;
		AsioContext mAsioContext;	
		long long mLastUpdateTime;
		std::thread  * mNetWorkThread;
		std::string mSrvConfigDirectory;
		asio::io_context::work mAsioWork;
		TimeRecorder mMainLogicTimeRecorder;
		std::vector<shared_ptr<Manager>> mSortManagerArray;
		std::unordered_map<std::string, shared_ptr<Manager>> mManagerMap;
	private:
		static Applocation * mApplocation;
	};


	template<typename T>
	inline shared_ptr<T> Applocation::AddManager()
	{
		const char * name = TypeReflection<T>::Name;
		Manager * pManager = this->GetManager<T>();
		if (pManager != nullptr)
		{
			throw std::string("Add Manager Repeat");
			return nullptr;
		}

		Manager * pNewManager = make_shared<T>();
		if (pNewManager != nullptr)
		{
			pNewManager->Init(name);
			this->mSortManagerArray.push_back(pNewManager);
			this->mManagerMap.insert(std::make_pair(name, pNewManager));
			return static_cast<T *>(pNewManager);
		}
		return nullptr;
	}
	template<typename T>
	inline shared_ptr<T> Applocation::GetManager()
	{
		const std::string name = TypeReflection<T>::Name;
		auto iter = this->mManagerMap.find(name);
		if (iter != this->mManagerMap.end())
		{
			Manager * pManager = iter->second;
			return static_cast<T*>(pManager);
		}
		auto iter1 = this->mManagerMap.begin();
		for (; iter1 != this->mManagerMap.end(); iter1++)
		{
			T * pManager = dynamic_cast<T*>(iter1->second);
			if (pManager != nullptr)
			{
				return pManager;
			}
		}
		return nullptr;
	}
	template<typename T>
	inline shared_ptr<T> Applocation::GetOrAddManager()
	{
		shared_ptr<T> pManager = this->GetManager<T>();
		if (pManager == nullptr)
		{
			pManager = this->AddManager<T>();
		}
		return pManager;
	}
}