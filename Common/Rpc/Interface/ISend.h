//
// Created by leyi on 2023/6/7.
//

#ifndef APP_ISEND_H
#define APP_ISEND_H
#include"Rpc/Client/Message.h"

namespace Tendo
{
	class ISender
	{
	public:
		explicit ISender(int net) : mNet(net) { }
		int NetType() const { return this->mNet; }
		virtual int Send(const std::string & address, const std::shared_ptr<Msg::Packet> & message) = 0;
	private:
		int mNet;
	};
}
#endif //APP_ISEND_H
