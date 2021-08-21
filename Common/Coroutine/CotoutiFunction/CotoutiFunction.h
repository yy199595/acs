#pragma once

#include <functional>
#include <type_traits>

namespace Sentry {

	class Closure {
	public:
		Closure() = default;
		virtual ~Closure() = default;

		virtual void run() = 0;
	};

	namespace xx {

		template<typename F,typename T>
		class Method0 : public Closure {
		public:
			Method0(F f, T* o) : _f(f), _o(o) {}

			virtual void run() {
				(_o->*_f)();	
			}

		private:
			F _f;
			T* _o;

			virtual ~Method0() {}
		};

		template<typename F, typename T, typename P>
		class Method1 : public Closure {
		public:
			Method1(F&& f, T* o, P&& p)
				: _f(std::forward<F>(f)), _o(o), _p(std::forward<P>(p)) {
			}

			virtual void run() {
				(_o->*_f)(_p);		
			}

		private:
			typename std::remove_reference<F>::type _f;
			T* _o;
			typename std::remove_reference<P>::type _p;

			virtual ~Method1() {}
		};

		template<typename F, typename T, typename P1, typename P2>
		class Method2 : public Closure {
		public:
			Method2(F&& f, T* o, P1&& p1, P2 && p2)
				: _f(std::forward<F>(f)), _o(o), _p1(std::forward<P1>(p1)),
				_p2(std::forward<P2>(p2))
			{

			}

			virtual void run() {
				(_o->*_f)(_p1, _p2);			
			}

		private:
			typename std::remove_reference<F>::type _f;
			T* _o;
			typename std::remove_reference<P1>::type _p1;
			typename std::remove_reference<P2>::type _p2;

			virtual ~Method2() {}
		};

		template<typename F, typename T, typename P1, typename P2, typename P3>
		class Method2 : public Closure {
		public:
			Method2(F&& f, T* o, P1&& p1, P2 && p2, P3 && p3)
				: _f(std::forward<F>(f)), _o(o), _p1(std::forward<P1>(p1)),
				_p2(std::forward<P2>(p2)),_p3(std::forward<P3>(p3))
			{

			}

			virtual void run() {
				(_o->*_f)(_p1, _p2, _p3);
			}

		private:
			typename std::remove_reference<F>::type _f;
			T* _o;
			typename std::remove_reference<P1>::type _p1;
			typename std::remove_reference<P2>::type _p2;
			typename std::remove_reference<P3>::type _p3;

			virtual ~Method2() {}
		};

	} // xx

	template<typename F,typename T>
	inline Closure* new_closure(F && f, T* o) {
		return new xx::Method0<T>(std::forward<F>(f), o);
	}

	template<typename F, typename T, typename P>
	inline Closure* new_closure(F&& f, T* o, P&& p) {
		return new xx::Method1<F, T, P>(std::forward<F>(f), o, std::forward<P>(p));
	}

	template<typename F, typename T, typename P1, typename P2>
	inline Closure* new_closure(F&& f, T* o, P1&& p1, P2 && p2) {
		return new xx::Method2<F, T, P1, P2>(std::forward<F>(f), o, std::forward<P1>(p1), std::forward<P2>(p2));
	}

} // co
