#pragma once

#include <memory>
#include <functional>
#include <type_traits>

#ifdef __SHARE_PTR_COUNTER__
#include "Core/Memory/MemoryObject.h"
#endif

namespace acs
{

	class StaticMethod
#ifdef __SHARE_PTR_COUNTER__
	: public memory::Object<StaticMethod>
#endif
	{
	 public:
		StaticMethod() = default;

		virtual ~StaticMethod() = default;

		virtual void run() = 0;
	};

	class LambdaMethod : public StaticMethod
	{
	 public:
		explicit LambdaMethod(std::function<void(void)>&& func)
			: mFunc(func)
		{
		}
		inline void run() final
		{
			this->mFunc();
		}

	 private:
		std::function<void()> mFunc;
	};

	template<typename F, typename T>
	class StaticMethod0 : public StaticMethod
	{
	 public:
		StaticMethod0(F&& f, T* o)
			: _o(o), _func(std::forward<F>(f))
		{
		}

		inline void run() final
		{
			(_o->*_func)();
		}
		~StaticMethod0() final = default;
	 private:
		T* _o;
		F _func;

	};

	template<typename F, typename T, typename P>
	class StaticMethod1 final : public StaticMethod
	{
	 public:
		StaticMethod1(F&& f, T* o, P&& p)
			: _o(o), _func(std::forward<F>(f)), _p(std::forward<P>(p))
		{
		}

		inline void run() final
		{
			(_o->*_func)(_p);
		}
		~StaticMethod1() final = default;
	 private:
		T* _o;
		F _func;
		typename std::remove_reference<P>::type _p;

	};

	template<typename F, typename T, typename P1, typename P2>
	class StaticMethod2 : public StaticMethod
	{
	 public:
		StaticMethod2(F&& f, T* o, P1&& p1, P2&& p2)
			: _o(o), _func(std::forward<F>(f)), _p1(std::forward<P1>(p1)),
			  _p2(std::forward<P2>(p2))
		{

		}

		inline void run() final
		{
			(_o->*_func)(_p1, _p2);
		}
		~StaticMethod2() final = default;

	 private:
		T* _o;
		F _func;
		typename std::remove_reference<P1>::type _p1;
		typename std::remove_reference<P2>::type _p2;

	};

	template<typename F, typename T, typename P1, typename P2, typename P3>
	class StaticMethod3 : public StaticMethod
	{
	 public:
		StaticMethod3(F&& f, T* o, P1&& p1, P2&& p2, P3&& p3)
			: _o(o), _func(std::forward<F>(f)), _p1(std::forward<P1>(p1)),
			  _p2(std::forward<P2>(p2)), _p3(std::forward<P3>(p3))
		{

		}

		inline void run() final
		{
			(_o->*_func)(_p1, _p2, _p3);
		}
		~StaticMethod3() final = default;
	 private:
		T* _o;
		F _func;
		typename std::remove_reference<P1>::type _p1;
		typename std::remove_reference<P2>::type _p2;
		typename std::remove_reference<P3>::type _p3;

	};

	template<typename T, typename F>
	inline std::unique_ptr<StaticMethod> NewMethodProxy(F f, T* o)
	{
		return std::make_unique<StaticMethod0<F, T>>(std::forward<F>(f), o);
	}

	template<typename F, typename T, typename P>
	inline std::unique_ptr<StaticMethod> NewMethodProxy(F f, T* o, P&& p)
	{
		return  std::make_unique<StaticMethod1<F, T, P>>(
			std::forward<F>(f), o, std::forward<P>(p));
	}

	template<typename F, typename T, typename P1, typename P2>
	inline std::unique_ptr<StaticMethod> NewMethodProxy(F f, T* o, P1&& p1, P2&& p2)
	{
		return  std::make_unique<StaticMethod2<F, T, P1, P2>>(
			std::forward<F>(f), o, std::forward<P1>(p1), std::forward<P2>(p2));
	}

	template<typename F, typename T, typename P1, typename P2, typename P3>
	inline std::unique_ptr<StaticMethod> NewMethodProxy(F f, T* o, P1&& p1, P2&& p2, P3&& p3)
	{
		return  std::make_unique<StaticMethod3<F, T, P1, P2, P3>>(
			std::forward<F>(f), o, std::forward<P1>(p1), std::forward<P2>(p2), std::forward<P3>(p3));
	}
} // co

