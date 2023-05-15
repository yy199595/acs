//
// Created by leyi on 2023/5/15.
//

#ifndef APP_NETUNIT_H
#define APP_NETUNIT_H

#include"Unit.h"
#include<google/protobuf/message.h>

namespace Msg
{
	class Packet;
}
namespace pb = google::protobuf;
namespace Tendo
{
	class NetUnit : public Unit
	{
	public:
		using Unit::Unit;
	public:
		virtual bool GetAddr(const std::string & server, int & targetId) = 0;
		virtual bool GetAddr(const std::string & server, std::string & addr) = 0;
	protected:
		int NewRequest(const std::string & func, const pb::Message * request, std::shared_ptr<Msg::Packet> & message);
	protected:
		bool LateAwake() override;
	protected:
		class InnerNetComponent * mInnerComponent;
		class LocationComponent * mLocationComponent;
	};
}


#endif //APP_NETUNIT_H
