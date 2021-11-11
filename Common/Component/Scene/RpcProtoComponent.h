#pragma once

#include <Component/Component.h>
#include <Other/ProtocolConfig.h>

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
        const ProtocolConfig *GetProtocolConfig(const std::string & fullName) const;
        bool GetMethods(const std::string & service, std::vector<std::string> & methods);

        Message * CreateResquest(const std::string & fullName);
        Message * CreateResponse(const std::string & fullName);

    public:
        Message * NewProtoMessage(const std::string & name);
    private:
        bool AddProto(const std::string & name);
    private:
        std::mutex mLock;
        std::unordered_map<std::string, const Message *> mProtoMap;
        std::unordered_map<std::string, ProtocolConfig> mProtocolNameMap;
		std::unordered_map<std::string, std::vector<ProtocolConfig>> mServiceMap;

    };
}// namespace GameKeeper