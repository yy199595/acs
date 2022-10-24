//
// Created by yjz on 2022/10/24.
//

#ifndef _SUBPUBLISH_H_
#define _SUBPUBLISH_H_
#include"Service/LocalRpcService.h"

namespace Sentry
{
	class SubPublish : public LocalRpcService
	{
	 public:
		SubPublish() = default;
	 private:
		XCode Sub(const Rpc::Head & head, const s2s::forward::sub & request);
		XCode UnSub(const Rpc::Head & head, const s2s::forward::unsub & request);
		XCode Publish(const Rpc::Head & head, const s2s::forward::unsub & request);
	 private:
		bool OnStart() final;
		bool OnClose() final;
	};
}

#endif //_SUBPUBLISH_H_
