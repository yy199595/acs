#pragma once

#ifndef ASIO_STANDALONE
#define ASIO_STANDALONE
#endif
#include<Util/TimeHelper.h>
#include<Define/CommonDef.h>
#include<Other/TimeRecorder.h>
#include<Global/ServerConfig.h>
#include<Define/CommonTypeDef.h>
#include<Define/ClassStatement.h>
#include<Manager/ManagerInterface.h>
using namespace std;
using namespace asio::ip;

namespace SoEasy
{
	class Manager;
	class ServiceBase;
	class ThreadPool;
	class Applocation
	{
	public:
		Applocation(const std::string srvName, const std::string configPath);
		virtual ~Applocation() {};
	public:
		ThreadPool * GetThreadPool() { return mThreadPool; }
		ServerConfig & GetConfig() { return this->mConfig; }
		inline float GetDelaTime() { return this->mDelatime; }
		inline long long GetLogicTime() { return this->mLogicTime; }
		inline long long GetStartTime() { return this->mStartTime; }
		const std::string & GetServerName() { return this->mServerName; }
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
		void GetManagers(std::vector<Manager *> & managers);
	private:
		bool LoadManager();
		bool InitManager();
	public:
		int Run();
		int Stop();
		float GetMeanFps();
	private:
		void UpdateConsoleTitle();
		bool GetTypeName(size_t hash, std::string & name);
	private:
		int LogicMainLoop();
	private:
		bool mIsClose;
		long long mLogicTime;
		long long mStartTime;
		ServerConfig mConfig;
		std::string mServerName;
		long long mLastUpdateTime;
		long long mLastSystemTime;
		class LogHelper * mLogHelper;
		std::string mSrvConfigDirectory;
	private:
		float mLogicFps;
		float mDelatime;
		float mSystemFps;
	private:	
		long long mLogicRunCount;
		long long mSystemRunCount;
		long long mMainLoopStartTime;
	private:	
		static Applocation * mApplocation;
		std::unordered_map<std::string, Manager *> mManagerMap;
	private:
		std::vector<IFrameUpdate *> mFrameUpdateManagers;
		std::vector<ISystemUpdate *> mSystemUpdateManagers;
		std::vector<ISecondUpdate *> mSecondUpdateManagers;
		std::vector<ILastFrameUpdate *> mLastFrameUpdateManager;
	};
	template<typename T>
	inline bool Applocation::AddManager()
	{
		std::string name;
		size_t hash = typeid(T).hash_code();
		if (!this->GetTypeName(hash, name))
		{
			SayNoDebugError("please register type:" << typeid(T).name());
			return false;
		}
		return this->AddManager(name);
	}
	template<typename T>
	inline T * Applocation::GetManager()
	{
		std::string name;
		size_t hash = typeid(T).hash_code();
		if (!this->GetTypeName(hash, name))
		{
			SayNoDebugError("use 'TYPE_REFLECTION' register type:" << typeid(T).name());
			return nullptr;
		}
		auto iter = this->mManagerMap.find(name);
		if (iter != this->mManagerMap.end())
		{
			return dynamic_cast<T*>(iter->second);
		}
		auto iter1 = this->mManagerMap.begin();
		for (; iter1 != this->mManagerMap.end(); iter1++)
		{
			T * manager = dynamic_cast<T*>(iter1->second);
			if (manager != nullptr)
			{
				return manager;
			}
		}
		return nullptr;
	}
	template<typename T>
	inline bool Applocation::TryAddManager()
	{
		std::string name;
		size_t hash = typeid(T).hash_code();
		if (!this->GetTypeName(hash, name))
		{
			SayNoDebugError("player register" << typeid(T).name());
			return false;
		}
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