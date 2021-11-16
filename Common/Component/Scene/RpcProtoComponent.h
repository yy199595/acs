#pragma once
#include <XCode/XCode.h>
#include <Component/Component.h>
#include <Other/ProtocolConfig.h>
#include <shared_mutex>
namespace GameKeeper
{
    struct CodeConfig
    {
    public:
        int Code;
        std::string Name;
        std::string Desc;
    };
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
		bool HasServiceMethod(const std::string & service, const std::string & method);
        bool GetMethods(const std::string & service, std::vector<std::string> & methods);
    public:
        const CodeConfig * GetCodeConfig(int code) const;
        const ProtocolConfig *GetProtocolConfig(int methodId) const;
        const ProtocolConfig *GetProtocolConfig(const std::string & fullName) const;
        void DebugCode(XCode code);
    private:
        bool LoadCodeConfig();
    private:
        std::mutex mLock;
        std::unordered_map<int, CodeConfig> mCodeDescMap;
        std::unordered_map<int, ProtocolConfig> mProtocolIdMap;
        std::unordered_map<std::string, ProtocolConfig> mProtocolNameMap;
		std::unordered_map<std::string, std::vector<ProtocolConfig>> mServiceMap;

    };

#define GKDebugCode(code) { App::Get().GetComponent<RpcProtoComponent>()->DebugCode(code); }
}// namespace GameKeeper