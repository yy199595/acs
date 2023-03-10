#include"Message/s2s.pb.h"
#include"Service/PhysicalService.h"
namespace Sentry
{
	class WatchDog : public PhysicalService
	{
	public:
		WatchDog() = default;
	private:
		bool OnStart() final;
		bool OnClose() final;
	private:
		int Ping(const Rpc::Head& message);
		int ShowLog(const s2s::log::show& request);
	};
}