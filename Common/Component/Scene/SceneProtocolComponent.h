#pragma once

#include <Component/Component.h>
#include <Other/ProtocolConfig.h>

namespace Sentry
{
    class SceneProtocolComponent : public Component
    {
    public:
		SceneProtocolComponent();

        ~SceneProtocolComponent() {}

    protected:
        bool Awake() final;
		void Start() final;
    public:
		void GetServices(std::vector<std::string> & services);
		bool GetMethods(const std::string service, std::vector<std::string> & methods);
        const ProtocolConfig *GetProtocolConfig(unsigned short id) const;

        const ProtocolConfig *GetProtocolConfig(const std::string &service, const std::string &method) const;

    public:
        bool DestoryMessage(Message * message);

        Message *CreateMessage(const std::string &name);

        Message *CreateMessageByJson(const std::string &name, const char *msg, const size_t size);

	public:
		bool GetJsonByMessage(Message * message, std::string & json);
    private:
        std::unordered_map<unsigned short, ProtocolConfig *> mProtocolMap;
        std::unordered_map<std::string, ProtocolConfig *> mProtocolNameMap;
		std::unordered_map<std::string, std::vector<ProtocolConfig *>> mServiceMap;
    private:
        std::unordered_map<std::string, std::queue<Message *>> mProtocolPoolMap;
    };
}// namespace Sentry