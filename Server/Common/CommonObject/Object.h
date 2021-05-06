#pragma once
#include<memory>
#include<CommonCore/Applocation.h>
#include<CommonDefine/CommonTypeDef.h>


namespace SoEasy
{
	class Object
	{
	public:
		Object();
		virtual ~Object();
	public:
		void Init(Applocation * app, std::string & typeName);
		inline bool IsActive() { return this->mIsActive; }
		inline Applocation * GetApp() { return mAppLocation; }
		inline void SetActive(bool isActive) { this->mIsActive = isActive; }
		inline const std::string & GetTypeName() { return this->mClassName; }
	public:
		shared_ptr<class CoroutineManager> GetScheduler() { return this->mCoroutineManager; }
	public:
		virtual bool IsManager() { return false; }
		virtual bool IsComponent() { return false; }
		virtual bool IsGameObejct() { return false; }
	protected:
		virtual void OnAwake() { }
		virtual void OnDestory() { }
	private:
		bool mIsActive;
		std::string mClassName;
		Applocation * mAppLocation;
		shared_ptr<class CoroutineManager> mCoroutineManager;
	};
}

