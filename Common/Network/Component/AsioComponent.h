//
// Created by yjz on 2023/4/8.
//

#ifndef _ASIOCOMPONENT_H_
#define _ASIOCOMPONENT_H_
#include"Rpc/Client/Message.h"
#include"Core/Component/Component.h"
namespace Tendo
{
	class AsioComponent : public Component, public IRpc<Rpc::Packet>
	{
	 public:
		virtual int Send(const std::string & address, const std::shared_ptr<Rpc::Packet> & message) = 0;
		virtual int Send(const std::string & address, int code, const std::shared_ptr<Rpc::Packet> & message) = 0;
	};
}

#endif //_ASIOCOMPONENT_H_
