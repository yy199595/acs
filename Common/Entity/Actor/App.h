#pragma once
#include"Network/Tcp/Asio.h"
#include"Core/Singleton/Singleton.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Config/ServiceConfig.h"
#include"Timer/Component/TimerComponent.h"
#include"Async/Component/CoroutineComponent.h"
#include"Entity/Component/ActorMgrComponent.h"
#include"Log/Component/LogComponent.h"
#include"Entity/Actor/ServerActor.h"
namespace Tendo
{
	enum class ServerStatus
	{
		Init,
		Ready, //启动完成
		Running, // 正在运行
		Closing	 //正在关闭
	};
}
struct lua_State;
namespace Tendo
{
	class RpcService;
	class ProtoComponent;
	class App final : public ServerActor, public Singleton<App>
	{
	 public:
		explicit App(ServerConfig * config);
	 public:
        inline float GetFps() const { return this->mLogicFps; }
		inline bool IsStartDone() const { return this->mIsStartDone; }
		inline LogComponent* GetLogger() { return this->mLogComponent; }
		inline Asio::Context & MainThread() { return *this->mMainContext; }
		inline TimerComponent* GetTimer() { return this->mTimerComponent; }
		inline const ServerConfig * Config() const { return this->mConfig; }
		inline ProtoComponent * GetProto() { return this->mMessageComponent; }
		inline ActorMgrComponent * ActorMgr() { return this->mActorComponent; }
		inline CoroutineComponent* GetCoroutine() { return this->mTaskComponent; }
		inline bool IsMainContext(const Asio::Context * io) const { return this->mMainContext.get() == io;}
	 public:
		int Run();
		void Stop(int signum);
        bool OnDelComponent(Component *component) final { return false; }
        inline bool IsMainThread() const { return this->mThreadId == std::this_thread::get_id();}
    private:
#ifdef __OS_WIN__
		void UpdateConsoleTitle();
#endif
		bool LoadComponent();
		bool InitComponent();
		void StartAllComponent();
	 private:
        int mTickCount;
		float mLogicFps;
        bool mIsStartDone;
		ServerStatus mStatus;
		ServerConfig * mConfig;
        std::thread::id mThreadId;
        const long long mStartTime;
		LogComponent* mLogComponent;
		TimerComponent* mTimerComponent;
		CoroutineComponent* mTaskComponent;
		ProtoComponent * mMessageComponent;
		ActorMgrComponent * mActorComponent;
		std::unique_ptr<Asio::Context> mMainContext;
    };
}// namespace Sentry