#include<queue>
#include"Component/Component.h"

typedef std::unique_ptr<s2s::log::show> LogPtr;
namespace Sentry
{
	class WatchDogComponent : public Component, public IFrameUpdate
	{
	public:
		WatchDogComponent() = default;
	public:
		void ShowLog(spdlog::level::level_enum lv, const std::string& log);
	private:
		bool LateAwake() final;
		void OnFrameUpdate(float t) final;
	private:
		std::string mAddress;
		std::queue<LogPtr> mLogs;
	};
}