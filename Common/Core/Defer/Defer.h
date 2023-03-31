//
// Created by yjz on 2022/10/24.
//

#ifndef _DEFER_H_
#define _DEFER_H_
#include<functional>
#include"Rpc/Method/MethodProxy.h"
namespace Sentry
{
	class Defer
	{
	 public:
		Defer(std::function<void(void)> && func) : mFunc(std::move(func)) { }
		~Defer() { if(this->mFunc) { this->mFunc(); } }
	 public:
		void Cancle() { this->mFunc = nullptr; }
	 private:
		std::function<void(void)> mFunc;
	};
}

#endif //_DEFER_H_
