#pragma once
#include"Network/Tcp/Asio.h"
#include"Entity/Unit/Server.h"
#include"Server/Config/ServerConfig.h"
#include"Server/Config/ServerPath.h"
#include"Core/Singleton/Singleton.h"
#include"Server/Config/ServiceConfig.h"
#include"Async/Component/CoroutineComponent.h"
#include"Timer/Component/TimerComponent.h"
#include"Log/Component/LogComponent.h"

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

namespace Tendo
{
	class RpcService;
	class ProtoComponent;
    class App final : public Server, public Singleton<App>
	{
	 public:
		explicit App();
	 public:
        inline float GetFps() const { return this->mLogicFps; }
		inline LogComponent* GetLogger() { return this->mLogComponent; }
		inline Asio::Context & MainThread() { return *this->mMainContext; }
		inline TimerComponent* GetTimer() { return this->mTimerComponent; }
		inline ProtoComponent * GetProto() { return this->mMessageComponent; }
		inline CoroutineComponent* GetCoroutine() { return this->mTaskComponent; }
		inline bool IsMainContext(const Asio::Context * io) const { return this->mMainContext.get() == io;}
	 public:
		void Stop(int signum);
        int Run(int argc, char ** argv);
        bool OnDelComponent(Component *component) final { return false; }
        inline bool IsMainThread() const { return this->mThreadId == std::this_thread::get_id();}
    private:
#ifdef __OS_WIN__
		void UpdateConsoleTitle();
#endif
		bool LoadComponent();
		void StartAllComponent();
	 private:
        int mTickCount;
		float mLogicFps;
        bool mIsStartDone;
		ServerStatus mStatus;
        std::thread::id mThreadId;
        const long long mStartTime;
		LogComponent* mLogComponent;
		TimerComponent* mTimerComponent;
		CoroutineComponent* mTaskComponent;
		ProtoComponent * mMessageComponent;
        std::unique_ptr<Asio::Context> mMainContext;
    };
}// namespace Sentry