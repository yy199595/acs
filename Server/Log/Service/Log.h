//
// Created by yjz on 2023/3/2.
//

#ifndef _LOG_H_
#define _LOG_H_
#include"Service/PhysicalService.h"
namespace Sentry
{
	class Log : public PhysicalService
	{
	 public:
		Log() = default;
	 private:
		XCode Push(const s2s::log::push & request);
	 protected:
		void Init() final;
		bool OnStart() final;
		bool OnClose() final;
	};
}

#endif //_LOG_H_
