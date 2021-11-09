#pragma once

#include <Component/Component.h>
#include <Other/ProtocolConfig.h>

namespace GameKeeper
{
    class ProtocolComponent : public Component
    {
    public:
		ProtocolComponent() = default;
        ~ProtocolComponent() override = default;

    protected:
        bool Awake() final;
    public:
		bool HasService(const std::string & service);
		void GetServices(std::vector<std::string> & services);
		bool GetMethods(const std::string & service, std::vector<std::string> & methods);
        const ProtocolConfig *GetProtocolConfig(unsigned short id) const;

        const ProtocolConfig *GetProtocolConfig(const std::string &service, const std::string &method) const;

        //const HttpServiceConfig * GetHttpConfig(const std::string & path) const;

    private:
        bool LoadTcpServiceConfig(const std::string & path);
        //bool LoadHttpServiceConfig(const std::string & path);
    private:
        std::unordered_map<unsigned short, ProtocolConfig *> mProtocolMap;
        std::unordered_map<std::string, ProtocolConfig *> mProtocolNameMap;
        //std::unordered_map<std::string, HttpServiceConfig *> mHttpConfigMap;
		std::unordered_map<std::string, std::vector<ProtocolConfig *>> mServiceMap;
    };
}// namespace GameKeeper