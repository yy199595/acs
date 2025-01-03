//
// Created by 64658 on 2025/1/3.
//

#ifndef APP_COMMON_H
#define APP_COMMON_H
#include "Rpc/Client/Message.h"

namespace acs
{
	class IGate
	{
	public:
		virtual char GetNet() const = 0;
		virtual int Send(int id, rpc::Message * message) = 0;
		virtual void Broadcast(rpc::Message * message) = 0;
	};
}
#endif //APP_COMMON_H
