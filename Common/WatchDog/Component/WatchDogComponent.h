#include<queue>
#include<mutex>
#include"Component/Component.h"

typedef std::unique_ptr<s2s::log::show> LogPtr;
namespace Sentry
{
	class WatchDogComponent final : public Component
	{
	public:
		WatchDogComponent();
	public:
		void ShowLog(spdlog::level::level_enum lv, const std::string& log);
	private:
		bool LateAwake() final;
		void ShowLogInWatchDog(s2s::log::show * log);
	private:
		std::mutex mMutex;
		std::string mAddress;
		std::queue<s2s::log::show *> mLogs;
		class InnerNetComponent* mInnerComponent;
	};
}