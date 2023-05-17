#include"Message/s2s/s2s.pb.h"
#include"Rpc/Service/RpcService.h"
namespace Tendo
{
	class WatchDog : public RpcService
	{
	public:
		WatchDog() = default;
	private:
		bool OnInit() final;
	private:
		int Ping(const Msg::Packet& packet);
		int ShowLog(const s2s::log::show& request);
	};
}