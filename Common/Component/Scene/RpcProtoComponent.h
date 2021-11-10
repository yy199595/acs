#pragma once

#include <Component/Component.h>
#include <Other/ProtocolConfig.h>

namespace GameKeeper
{
    class RpcProtoComponent : public Component
    {
    public:
		RpcProtoComponent() = default;
        ~RpcProtoComponent() override = default;

    protected:
        bool Awake() final;
    public:
		bool HasService(const std::string & service);
		void GetServices(std::vector<std::string> & services);
        const ProtocolConfig *GetProtocolConfig(unsigned short id) const;
        bool GetMethods(const std::string & service, std::vector<std::string> & methods);
        const ProtocolConfig *GetProtocolConfig(const std::string &service, const std::string &method) const;

    public:
        Message * NewProtoMessage(const std::string & name);
    private:
        bool AddProto(const std::string & name);
        bool LoadTcpServiceConfig(const std::string & path);
    private:
        std::mutex mLock;
        std::unordered_map<std::string, Message *> mProtoMap;
        std::unordered_map<unsigned short, ProtocolConfig *> mProtocolMap;
        std::unordered_map<std::string, ProtocolConfig *> mProtocolNameMap;
		std::unordered_map<std::string, std::vector<ProtocolConfig *>> mServiceMap;

    };
}// namespace GameKeeper