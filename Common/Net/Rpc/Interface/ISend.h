//
// Created by leyi on 2023/6/7.
//

#ifndef APP_ISEND_H
#define APP_ISEND_H
#include"Rpc/Client/Message.h"

namespace acs
{
	class ISender
	{
	public:
		explicit ISender(char net) : mNet(net) { }
		char NetType() const { return this->mNet; }
		virtual int Send(int id, rpc::Message * message) = 0;
	private:
		char mNet;
	};
}
#endif //APP_ISEND_H
