#include"WatchDog.h"
#include"Log/Debug.h"
namespace Sentry
{
    bool WatchDog::OnStart()
    {
        BIND_COMMON_RPC_METHOD(WatchDog::Ping);
        BIND_COMMON_RPC_METHOD(WatchDog::ShowLog);
        return true;
    }

    bool WatchDog::OnClose()
    {
        return false;
    }

    int WatchDog::Ping(const Rpc::Head& message)
    {
        return 0;
    }

    int WatchDog::ShowLog(const s2s::log::show& request)
    {
        int lv = request.level();
        const std::string& name = request.name();
        const std::string& content = request.content();
        Debug::Console((spdlog::level::level_enum)lv, fmt::format("[{0}] {1}", name, content));
        return XCode::Successful;
    }
}
