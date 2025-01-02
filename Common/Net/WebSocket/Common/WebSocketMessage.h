//
// Created by 64658 on 2025/1/2.
//

#ifndef APP_WEBSOCKETMESSAGE_H
#define APP_WEBSOCKETMESSAGE_H
#include"Proto/Message/IProto.h"
#include "WebSocket/Common/WebSocket.h"
namespace ws
{
	struct Header
	{
		bool mFin;
		bool mMask;
		unsigned char mRsv;
		unsigned char mOpCode;
		unsigned int mLength;
	};

	class Message : public tcp::IProto
	{
	public:
		Message();
	public:
		void Clear() final;
		int OnSendMessage(std::ostream &os) final;
		int OnRecvMessage(std::istream &os, size_t size) final;
	public:
		inline void SetBody(unsigned char opcode) {
			this->mHeader.mOpCode = opcode;
		}
		inline void SetBody(unsigned char opcode, const std::string & message) {
			this->mHeader.mOpCode = opcode; this->mMessage = message;
		}
	public:
		const Header & GetHeader() const { return this->mHeader; }
		const std::string & GetMessageBody() const { return this->mMessage; }
	private:
		Header mHeader;
		char mMaskingKey[4];
		std::string mMessage;
	};
}


#endif //APP_WEBSOCKETMESSAGE_H
