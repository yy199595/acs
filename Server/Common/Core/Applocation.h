#pragma once

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif
#include<thread>
#include<Util/TimeHelper.h>
#include<Define/CommonDef.h>
#include<Other/TimeRecorder.h>
#include<Global/ServerConfig.h>
#include<Define/CommonTypeDef.h>
#include<Define/ClassStatement.h>
using namespace std;
using namespace asio::ip;

namespace SoEasy
{
	class Manager;
	class ManagerFactory;
	class Applocation
	{
	public:
		Applocation(const std::string srvName, ManagerFactory & factory, const std::string configPath);
		virtual ~Applocation() {};
	public:
		inline float GetFps() { return this->mFps; }
		ServerConfig & GetConfig() { return this->mConfig; }
		inline float GetDelaTime() { return this->mDelatime; }
		inline long long GetLogicTime() { return this->mLogicTime; }
		inline long long GetStartTime() { return this->mStartTime; }
		AsioContext & GetAsioContext() { return (*mAsioContext); }
		AsioContext * GetAsioContextPtr() { return mAsioContext; }
		long long GetRunTime() { return TimeHelper::GetSecTimeStamp() - this->mStartTime; }
		inline const std::string & GetConfigDirectory() { return this->mSrvConfigDirectory; }
	public:
		static Applocation * Get() { return mApplocation; }
	public:
		template<typename T>
		bool AddManager();
		bool AddManager(const std::string & name);
		template<typename T>
		inline T * GetManager();
		template<typename T>
		inline bool TryAddManager();
	private:
		bool LoadManager();
		bool InitManager();
	public:
		int Run();
		int Stop();
		float GetMeanFps();
	private:
		void UpdateConsoleTitle();
	private:
		int  LogicMainLoop();
		void UpdateManager(float delatime);
	private:
		AsioWork * mAsioWork;
		std::thread * mNetThread;
		AsioContext * mAsioContext;
	private:
		float mFps;
		bool mIsClose;
		float mDelatime;
		long long mLogicTime;
		long long mStartTime;
		ServerConfig mConfig;
		long long mLastUpdateTime;
		Manager * mCurrentManager;	
		class LogHelper * mLogHelper;
		std::string mSrvConfigDirectory;
	private:
		ManagerFactory & mManagerFactory;
		static Applocation * mApplocation;
		std::vector<Manager *> mSortManagers;
		std::unordered_map<std::string, Manager *> mManagerMap;
	};
	template<typename T>
	inline bool Applocation::AddManager()
	{
		const std::string name = SoEasy::GetTypeName<T>();
		return this->AddManager(name);
	}
	template<typename T>
	inline T * Applocation::GetManager()
	{
		const std::string name = SoEasy::GetTypeName<T>();
		auto iter = this->mManagerMap.find(name);
		if (iter == this->mManagerMap.end())
		{
			return nullptr;
		}
		return dynamic_cast<T*>(iter->second);
	}
	template<typename T>
	inline bool Applocation::TryAddManager()
	{
		const std::string name = SoEasy::GetTypeName<T>();
		auto iter = this->mManagerMap.find(name);
		if (iter == this->mManagerMap.end())
		{
			return this->AddManager(name);
		}
		return false;
	}

	inline Applocation * GetApp() { return Applocation::Get(); }
	template<typename T>
	inline T * GetManager() { return Applocation::Get()->GetManager<T>(); }

}