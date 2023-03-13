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
	private:
		int Ping(const Rpc::Packet& packet);
		int ShowLog(const s2s::log::show& request);
	};
}