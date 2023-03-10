#include<queue>
#include"Message/s2s.pb.h"
#include"Component/Component.h"

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
		std::string mAddress;
		std::queue<s2s::log::show *> mLogs;
		class InnerNetComponent* mInnerComponent;
	};
}