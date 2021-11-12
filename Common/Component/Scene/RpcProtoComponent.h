#pragma once

#include <Component/Component.h>
#include <Other/ProtocolConfig.h>
#include <shared_mutex>
namespace GameKeeper
{
    class RpcProtoComponent : public Component, public ILoadConfig
    {
    public:
		RpcProtoComponent() = default;
        ~RpcProtoComponent() override = default;

    protected:
        bool Awake() final;
        bool OnLoadConfig() final;
    public:
		bool HasService(const std::string & service);
		void GetServices(std::vector<std::string> & services);
        bool GetMethods(const std::string & service, std::vector<std::string> & methods);
    public:
        const ProtocolConfig *GetProtocolConfig(int methodId) const;
        const ProtocolConfig *GetProtocolConfig(const std::string & fullName) const;
    private:
        std::mutex mLock;
        std::unordered_map<int, ProtocolConfig> mProtocolIdMap;
        std::unordered_map<std::string, ProtocolConfig> mProtocolNameMap;
		std::unordered_map<std::string, std::vector<ProtocolConfig>> mServiceMap;

    };
}// namespace GameKeeper