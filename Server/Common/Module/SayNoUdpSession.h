#pragma once
#include<Module/Component.h>
namespace SoEasy
{
	class SayNoUdpSession : public Component
	{
	public:
		SayNoUdpSession(GameObject *);
	public:
		void StartReceiveMsg();
		void SendPackage(const std::string & message);
		void SendPackage(const char * message, const size_t size);
		void InitUdpSession(const std::string & ip, const unsigned short port, bool isReceive = false);
	protected:
		void OnFrameStart() override;
	private:
		void ReceiveAllMessage(const size_t size, const unsigned int packageNum);
		inline void ClearBuffer(char * buffer) { memset(buffer, 0, 1024); }
	private:
		bool mIsInitEndpoint;
		bool mIsReceiveMsg;
		unsigned int mPackageNumberof;
		AsioUdpSocket * mUdpSocket;
		AsioUdpEndPoint *  mRemotePoint;
		char udpSendMsgBuffer[1024];
		char udpReceiveBuffer[1024];
	};
}