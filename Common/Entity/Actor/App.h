#pragma once
#include"Network/Tcp/Asio.h"
#include<Core/Queue/TaskQueue.h>
#include"Core/Singleton/Singleton.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Async/Component/CoroutineComponent.h"
#include"Entity/Component/ActorComponent.h"
#include"Entity/Actor/Server.h"
namespace acs
{
	enum class ServerStatus
	{
		Init,
		Start, //正在启动
		Ready, //启动完成
		Running, // 正在运行
		Closing	 //正在关闭
	};

	namespace XServerCode
	{
		constexpr int Ok = 0;
		constexpr int ConfError = 1; //配置错误
		constexpr int InitError = 2; //加载错误
	};
}

namespace acs
{
	class RpcService;
	class ProtoComponent;
	class App final : public Server, public Singleton<App>
	{
	 public:
		explicit App(int id, ServerConfig & config);
	 public:
		void Sleep(int ms = 1000);
        inline float GetFps() const { return this->mLogicFps; }
		inline long long StartTime() const { return this->mStartTime; }
		inline const ServerConfig & Config() const { return mConfig; }
		inline ServerStatus GetStatus() const { return this->mStatus; }
		inline int GetTickCount() const { return this->mTickCount; }
	 public:
		int Run() noexcept;
		void Stop();
		bool Hotfix();
		bool LoadLang() const;
		long long MakeGuid();
		std::string NewUuid();
		inline ServerConfig & GetConfig() { return this->mConfig; }
		unsigned int StartCoroutine(std::function<void()> && func);
		inline Asio::Context & GetContext() { return this->mContext; }
        bool OnDelComponent(Component *component) final { return false; }
	public:
		template<typename T>
		static inline T * Get() { return App::Inst()->GetComponent<T>(); }
		static inline ActorComponent * ActorMgr() { return App::Inst()->mActor; }
		static inline ProtoComponent * GetProto() { return App::Inst()->mProto; }
		static inline CoroutineComponent* Coroutine() { return App::Inst()->mCoroutine; }

	public:
		std::string Sign(json::w::Document & document);
		bool DecodeSign(const std::string & sign, json::r::Document & document);
    private:
		bool LoadComponent();
		bool InitComponent();
		void StartAllComponent();
	 private:
		int mGuidIndex;
        int mTickCount;
		float mLogicFps;
		ServerStatus mStatus;
		Asio::Context mContext;
		ServerConfig & mConfig;
		long long mLastGuidTime;
		long long mNextNewDayTime; //下次新的一天时间
		asio::signal_set mSignal;
        const long long mStartTime;
		ActorComponent * mActor;
		ProtoComponent * mProto;
		CoroutineComponent* mCoroutine;
    };
}// namespace Sentry