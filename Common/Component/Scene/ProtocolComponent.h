#pragma once

#include <Component/Component.h>
#include <Other/ProtocolConfig.h>

namespace Sentry
{
    class ProtocolComponent : public Component
    {
    public:
		ProtocolComponent();

        ~ProtocolComponent() {}

    protected:
        bool Awake() final;
		void Start() final;
    public:
		void GetServices(std::vector<std::string> & services);
		bool GetMethods(const std::string service, std::vector<std::string> & methods);
        const ProtocolConfig *GetProtocolConfig(unsigned short id) const;

        const ProtocolConfig *GetProtocolConfig(const std::string &service, const std::string &method) const;

    private:
        std::unordered_map<unsigned short, ProtocolConfig *> mProtocolMap;
        std::unordered_map<std::string, ProtocolConfig *> mProtocolNameMap;
		std::unordered_map<std::string, std::vector<ProtocolConfig *>> mServiceMap;
    };
}// namespace Sentry