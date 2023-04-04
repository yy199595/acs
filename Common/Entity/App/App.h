#pragma once
#include"Network/Tcp/Asio.h"
#include"Entity/Unit/Unit.h"
#include"Server/Config/ServerConfig.h"
#include"Util/Time/TimeHelper.h"
#include"Server/Config/ServerPath.h"
#include"Core/Singleton/Singleton.h"
#include"Server/Config/ServiceConfig.h"
#include"Async/Component/AsyncMgrComponent.h"
#include"Timer/Component/TimerComponent.h"
#include"Log/Component/LogComponent.h"

namespace Sentry
{
	enum class ServerStatus
	{
		Init,
		Ready, //启动完成
		Running, // 正在运行
		Closing	 //正在关闭
	};
}

namespace Sentry
{
	class RpcService;
	class ProtoComponent;
    class App final : public Unit, public Singleton<App>
	{
	 public:
		explicit App();
	 public:
        inline float GetFps() const { return this->mLogicFps; }
		inline LogComponent* GetLogger() { return this->mLogComponent; }
		inline Asio::Context & MainThread() { return *this->mMainContext; }
		inline AsyncMgrComponent* GetTaskComponent() { return this->mTaskComponent; }
		inline TimerComponent* GetTimerComponent() { return this->mTimerComponent; }
		inline ProtoComponent * GetMsgComponent() { return this->mMessageComponent; }
	 public:
		void Stop();
        int Run(int argc, char ** argv);
		template<typename T>
		inline RpcService * GetService();
		inline RpcService * GetService(const std::string & name);
        bool OnDelComponent(Component *component) final { return false; }
        inline bool IsMainThread() const { return this->mThreadId == std::this_thread::get_id();}
    private:
#ifdef __OS_WIN__
		void UpdateConsoleTitle();
#endif
		bool LoadComponent();
		void WaitServerStart();
		void StartAllComponent();		
		static void HandleSignal(int signal);
	 private:
        int mTickCount;
		float mLogicFps;
        bool mIsStartDone;
		ServerStatus mStatus;
        std::thread::id mThreadId;
        const long long mStartTime;
        AsyncMgrComponent* mTaskComponent;
		LogComponent* mLogComponent;
		TimerComponent* mTimerComponent;
		ProtoComponent * mMessageComponent;
        std::unique_ptr<Asio::Context> mMainContext;
        std::unordered_map<std::string, RpcService*> mServiceMap;
    };

	inline RpcService* App::GetService(const std::string& name)
	{
		auto iter = this->mServiceMap.find(name);
		return iter != this->mServiceMap.end() ? iter->second : nullptr;
	}
	template<typename T>
	inline RpcService* App::GetService()
	{
		const std::string & name = ComponentFactory::GetName<T>();
		auto iter = this->mServiceMap.find(name);
		return iter != this->mServiceMap.end() ? iter->second : nullptr;
	}
}// namespace Sentry