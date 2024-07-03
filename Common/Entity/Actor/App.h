#pragma once
#include"Network/Tcp/Asio.h"
#include<Core/Queue/TaskQueue.h>
#include"Core/Singleton/Singleton.h"
#include"Server/Config/ServerConfig.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Async/Component/CoroutineComponent.h"
#include"Entity/Component/ActorComponent.h"
#include"Entity/Actor/Server.h"
namespace joke
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

namespace joke
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
		inline Asio::Context & GetContext() { return this->mContext; }
		inline long long StartTime() const { return this->mStartTime; }
		inline const ServerConfig & Config() const { return mConfig; }
		inline ServerStatus GetStatus() const { return this->mStatus; }
		inline ProtoComponent * GetProto() { return this->mMessageComponent; }
		inline ActorComponent * ActorMgr() { return this->mActorComponent; }
		inline CoroutineComponent* Coroutine() { return this->mTaskComponent; }
	 public:
		int Run();
		void Stop();
		bool Hotfix();
		bool LoadLang();
		long long MakeGuid();
		std::string NewUuid();
		unsigned int StartCoroutine(std::function<void()> && func);
        bool OnDelComponent(Component *component) final { return false; }
        inline bool IsMainThread() const { return this->mThreadId == std::this_thread::get_id();}

#ifdef __ENABLE_OPEN_SSL__
	public:
		std::string Sign(json::w::Document & document);
		bool DecodeSign(const std::string & sign, json::r::Document & document);
#endif
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
		asio::signal_set mSignal;
		std::thread::id mThreadId;
        const long long mStartTime;
		ActorComponent * mActorComponent;
		CoroutineComponent* mTaskComponent;
		ProtoComponent * mMessageComponent;
    };
}// namespace Sentry