#pragma once
#include <XCode/XCode.h>
#include <Component/Component.h>
#include <Other/ProtoConfig.h>
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
    class RpcConfigComponent : public Component, public ILoadConfig
    {
    public:
		RpcConfigComponent() = default;
        ~RpcConfigComponent() override = default;

    protected:
        bool Awake() final;
        bool LateAwake() final;
        bool OnLoadConfig() final;
    public:
		bool HasService(const std::string & service);		
		void GetServices(std::vector<std::string> & services);
		bool HasServiceMethod(const std::string & service, const std::string & method);
        bool GetMethods(const std::string & service, std::vector<std::string> & methods);
    public:
        const CodeConfig * GetCodeConfig(int code) const;
        const ProtoConfig *GetProtocolConfig(int methodId) const;
        const ProtoConfig *GetProtocolConfig(const std::string & fullName) const;

#ifdef __DEBUG__
        void DebugCode(XCode code);
        std::string GetCodeDesc(XCode code);
#endif
    private:
        bool LoadCodeConfig();
    private:
        std::mutex mLock;
        std::string mConfigFileMd5;
        std::unordered_map<int, CodeConfig> mCodeDescMap;
        std::unordered_map<int, ProtoConfig> mProtocolIdMap;
        std::unordered_map<std::string, ProtoConfig> mProtocolNameMap;
		std::unordered_map<std::string, std::vector<ProtoConfig>> mServiceMap;

    };

#define GKDebugCode(code) { App::Get().GetComponent<RpcConfigComponent>()->DebugCode(code); }
}// namespace GameKeeper