#pragma once
#include <Other/ObjectFactory.h>
#include <Pool/ObjectPool.h>
#include <Protocol/com.pb.h>
#include <XCode/XCode.h>

namespace GameKeeper
{
	template<typename T>
	using ServiceMethodType1 = XCode(T::*)();

	template<typename T>
	using ServiceMethodType11 = XCode(T::*)(long long);

	template<typename T, typename T1>
	using ServiceMethodType2 = XCode(T::*)(const T1 &);

	template<typename T, typename T1>
	using ServiceMethodType22 = XCode(T::*)(long long, const T1 &);

	template<typename T, typename T1, typename T2>
	using ServiceMethodType3 = XCode(T::*)(const T1 &, T2 &);

	template<typename T, typename T1, typename T2>
	using ServiceMethodType33 = XCode(T::*)(long long, const T1 &, T2 &);

	template<typename T, typename T1>
	using ServiceMethodType4 = XCode(T::*)(T1 &);

	template<typename T, typename T1>
	using ServiceMethodType44 = XCode(T::*)(long long, T1 &);

}// namespace GameKeeper


namespace GameKeeper
{
	
	class ServiceMethod
	{
	public:
		ServiceMethod(const std::string name) : mName(name) {}
	public:
		virtual bool IsLuaMethod() = 0;
        virtual void SetAddress(const std::string & address) { };
        virtual XCode Invoke(const com::Rpc_Request & request, std::string & response) = 0;
		const std::string & GetName() { return this->mName; }
	private:
		std::string mName;
	};


	template<typename T>
	class ServiceMethod1 : public ServiceMethod
	{
	public:
		ServiceMethod1(const std::string name, T * o, ServiceMethodType1<T> func)
			: ServiceMethod(name) ,_o(o), _func(func) { }

		ServiceMethod1(const std::string name, T * o, ServiceMethodType11<T> func)
			: ServiceMethod(name), _o(o), _objfunc(func) { }
	public:
        void SetAddress(const std::string & address) {_o->SetCurAddress(address); };

        XCode Invoke(const com::Rpc_Request & request, std::string & response) override
		{
			long long userId = request.userid();
            if (userId == 0) {
				return (_o->*_func)();
			}
			return (_o->*_objfunc)(userId);
		}

        bool IsLuaMethod() override { return false; };
	private:
		T * _o;
		ServiceMethodType1<T> _func;
		ServiceMethodType11<T> _objfunc;
	};
	template<typename T, typename T1>
	class ServiceMethod2 : public ServiceMethod
	{
	public:
		ServiceMethod2(const std::string name, T * o, ServiceMethodType2<T, T1> func)
			:ServiceMethod(name), _o(o), _func(func) { }

		ServiceMethod2(const std::string name, T * o, ServiceMethodType22<T, T1> func)
			:ServiceMethod(name), _o(o), _objfunc(func) { }
	public:
        void SetAddress(const std::string & address) {_o->SetCurAddress(address); };
        XCode Invoke(const com::Rpc_Request & request, std::string & response) override
        {
            T1 *req = mReqMessagePool.Create();
            if (!req->ParseFromString(request.messagedata()))
            {
                mReqMessagePool.Destory(req);
                return XCode::ParseMessageError;
            }
            XCode code = request.userid() == 0 ?
                         (_o->*_func)(*req) : (_o->*_objfunc)(request.userid(), *req);
            mReqMessagePool.Destory(req);
            return code;
        }
		bool IsLuaMethod() override { return false; };
	private:
		T * _o;
		ObjectPool<T1> mReqMessagePool;
		ServiceMethodType2<T, T1> _func;
		ServiceMethodType22<T, T1> _objfunc;
	};

	template<typename T, typename T1, typename T2>
	class ServiceMethod3 : public ServiceMethod
	{
	public:
		typedef XCode(T::*ServerFunc)(long long, const T1 &, T2 &);
		ServiceMethod3(const std::string name, T * o, ServiceMethodType3<T, T1, T2> func)
			: ServiceMethod(name),_o(o), _func(func) { }

		ServiceMethod3(const std::string name, T * o, ServiceMethodType33<T, T1, T2> func)
			: ServiceMethod(name), _o(o), _objfunc(func) { }
	public:
        void SetAddress(const std::string & address) {_o->SetCurAddress(address); };
        XCode Invoke(const com::Rpc_Request & request, std::string & response) override
		{
            T1 * req = mReqMessagePool.Create();
			if (!req->ParseFromString(request.messagedata()))
			{
				mReqMessagePool.Destory(req);
				return XCode::ParseMessageError;
			}
            T2 * res = mResMessagePool.Create();

			XCode code = request.userid() == 0 ?
				(_o->*_func)(*req, *res) : (_o->*_objfunc)(request.userid(), *req, *res);

			if (code == XCode::Successful)
			{
                res->SerializeToString(&response);
			}
			mReqMessagePool.Destory(req);
			mResMessagePool.Destory(res);
			return code;
		}
		bool IsLuaMethod() override { return false; };
	private:
		T * _o;
		ObjectPool<T1> mReqMessagePool;
		ObjectPool<T2> mResMessagePool;
		ServiceMethodType3<T, T1, T2> _func;
		ServiceMethodType33<T, T1, T2> _objfunc;
	};

	template<typename T, typename T1>
	class ServiceMethod4 : public ServiceMethod
	{
	public:
		ServiceMethod4(const std::string name, T * o, ServiceMethodType4<T, T1> func)
			:ServiceMethod(name), _o(o), _func(func) { }

		ServiceMethod4(const std::string name, T * o, ServiceMethodType44<T, T1> func)
			:ServiceMethod(name), _o(o), _objfunc(func) { }
	public:
        void SetAddress(const std::string & address) {_o->SetCurAddress(address); };
        XCode Invoke(const com::Rpc_Request & request, std::string & response) override
		{
            T1 * res = mResMessagePool.Create();

            XCode code = request.userid() == 0 ?
				(_o->*_func)(*res) : (_o->*_objfunc)(request.userid(), *res);

            if (code == XCode::Successful)
            {
                res->SerializeToString(&response);
            }
            mResMessagePool.Destory(res);

			return code;
		}
		bool IsLuaMethod() override { return false; };
	private:
		T * _o;
		ObjectPool<T1> mResMessagePool;
		ServiceMethodType4<T, T1> _func;
		ServiceMethodType44<T, T1> _objfunc;
	};
}