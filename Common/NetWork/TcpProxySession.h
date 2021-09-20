#pragma once


#include <Protocol/com.pb.h>
#include <XCode/XCode.h>
#include <NetWork/SocketEvent.h>

using namespace google::protobuf;
namespace Sentry
{
    class TcpProxySession
    {
    public:
        TcpProxySession(const std::string &address);

        TcpProxySession(const std::string &name, const std::string &address);

        ~TcpProxySession();

    public:
        const std::string &GetName() { return this->mName; }

        const std::string &GetAddress() { return this->mAddress; }

        bool IsNodeSession() { return this->mSessionType == SessionType::SessionNode; }

        int GetConnectCount() { return this->mConnectCount; }

    public:
		void StartColse();

        void StartConnect();
		void SetActive(bool active);
		bool IsActive() { return this->mIsActive; }	
        bool SendMessageData(PacketMapper *messageData);

    public:
        bool Notice(const std::string &service, const std::string &method);                        //不回应
        bool Notice(const std::string &service, const std::string &method, const Message &request);//不回应
    private:
		bool mIsActive;
        int mConnectCount;
        std::string mName;
        std::string mAddress;
        SessionType mSessionType;
        class NetSessionComponent *mNetManager;
    };
}// namespace Sentry