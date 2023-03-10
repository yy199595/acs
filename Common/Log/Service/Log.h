//
// Created by yjz on 2023/3/2.
//

#ifndef _LOG_H_
#define _LOG_H_
#include"Message/s2s.pb.h"
#include"Service/PhysicalService.h"
namespace Sentry
{
	class Log final : public PhysicalService
	{
	 public:
		Log() = default;
	 private:
		int Login(const s2s::log::login & request);
	 protected:
		void Init() final;
		bool OnStart() final;
		bool OnClose() final;
	};
}

#endif //_LOG_H_
