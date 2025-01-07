#pragma once
#include"XCode/XCode.h"
#include"Rpc/Common/Message.h"

using namespace google::protobuf;
namespace acs
{
	template<typename T>
	using ServiceMethodType1 = int(T::*)();

	template<typename T>
	using ServiceMethodType11 = int(T::*)(long long);

	template<typename T, typename T1>
	using ServiceMethodType2 = int(T::*)(const T1 &);

	template<typename T, typename T1>
	using ServiceMethodType22 = int(T::*)(long long, const T1 &);

	template<typename T, typename T1, typename T2>
	using ServiceMethodType3 = int(T::*)(const T1 &, T2 &);

	template<typename T, typename T1, typename T2>
	using ServiceMethodType33 = int(T::*)(long long, const T1 &, T2 &);

	template<typename T, typename T1>
	using ServiceMethodType34 = int(T::*)(const T1 &, rpc::Message &);

	template<typename T, typename T1>
	using ServiceMethodType4 = int(T::*)(T1 &);

	template<typename T, typename T1>
	using ServiceMethodType44 = int(T::*)(long long, T1 &);

	template<typename T>
	using ServiceMethodType6 = int(T::*)(const rpc::Message & packet);

	template<typename T>
	using ServiceMethodType7 = int(T::*)(const rpc::Message& request, rpc::Message & response);

	template<typename T>
	using ServiceMethodType8 = int(T::*)(const json::r::Document& request);

	template<typename T>
	using ServiceMethodType9 = int(T::*)(const json::r::Document& request, json::w::Document & response);

//    template<typename T, typename T1>
//    using ServiceMethodType5 = int(T::*)(const Rpc::Head & head, const T1 &);
//
//
//    template<typename T>
//    using ServiceMethodType7 = int(T::*)(const Rpc::Head & head, Rpc::Head &);


//	template<typename T, typename T1, typename T2>
//	using ServiceMethodType55 = XCode(T::*)(const std::string & address, const T1 &, T2 &);

}// namespace Sentry


namespace acs
{

	class ServiceMethod
	{
	 public:
		explicit ServiceMethod(std::string name)
			: mName(std::move(name)) {}
			virtual ~ServiceMethod() = default;
	 public:
		virtual int Invoke(rpc::Message & message) = 0;
		const std::string& GetName()
		{
			return this->mName;
		}
	 private:
		std::string mName;
	};

	template<typename T>
	class ServiceMethod1 final : public ServiceMethod
	{
	 public:
		ServiceMethod1(const std::string name, T* o, ServiceMethodType1<T> func, std::string key)
			: ServiceMethod(name), _o(o), mHasUserId(false), mKey(std::move(key)), _func(func)
		{
		}

		ServiceMethod1(const std::string name, T* o, ServiceMethodType11<T> func, std::string key)
			: ServiceMethod(name), _o(o), mHasUserId(true), mKey(std::move(key)), _objfunc(func)
		{
		}
	 public:

		int Invoke(rpc::Message & message) final
		{
			if (!this->mHasUserId)
			{
				return (_o->*_func)();
			}
            long long userId = 0;
            if(!message.GetHead().Get(this->mKey, userId))
            {
                return XCode::CallArgsError;
            }
			return (_o->*_objfunc)(userId);
		}
	 private:
		T* _o;
		const bool mHasUserId;
		const std::string mKey;
		ServiceMethodType1<T> _func;
		ServiceMethodType11<T> _objfunc;
	};
	template<typename T, typename T1>
	class ServiceMethod2 final : public ServiceMethod
	{
	 public:
		ServiceMethod2(const std::string name, T* o, ServiceMethodType2<T, T1> func, std::string key)
			: ServiceMethod(name), _o(o),mHasUserId(false), mKey(std::move(key)), _func(func)
		{
		}

		ServiceMethod2(const std::string name, T* o, ServiceMethodType22<T, T1> func, std::string key)
			: ServiceMethod(name), _o(o), mHasUserId(true), mKey(std::move(key)), _objfunc(func) { }
	 public:
		int Invoke(rpc::Message & message) override
		{
            std::unique_ptr<T1> request = std::make_unique<T1>();
            if(!message.ParseMessage(request.get()))
            {
                return XCode::CallArgsError;
            }
            message.Body()->clear();
            if (!this->mHasUserId)
			{
				return (_o->*_func)(*request);
			}
            long long userId = 0;
            if(!message.GetHead().Get(this->mKey, userId))
            {
                return XCode::CallArgsError;
            }
			return (_o->*_objfunc)(userId, *request);
		}
	 private:
		T* _o;
		const bool mHasUserId;
		const std::string mKey;
		ServiceMethodType2<T, T1> _func;
		ServiceMethodType22<T, T1> _objfunc;
	};

	template<typename T, typename T1, typename T2>
	class ServiceMethod3 final : public ServiceMethod
	{
	 public:
		typedef int(T::*ServerFunc)(long long, const T1&, T2&);
		ServiceMethod3(const std::string name, T* o, ServiceMethodType3<T, T1, T2> func, std::string key)
			: ServiceMethod(name), _o(o),mHasUserId(false), mKey(std::move(key)), _func(func)
		{
		}

		ServiceMethod3(const std::string name, T* o, ServiceMethodType33<T, T1, T2> func, std::string key)
			: ServiceMethod(name), _o(o), mHasUserId(true), mKey(std::move(key)), _objfunc(func)
		{
		}
	 public:
		int Invoke(rpc::Message & message) override
		{
            std::unique_ptr<T1> request = std::make_unique<T1>();
            std::unique_ptr<T2> response = std::make_unique<T2>();
            if(!message.ParseMessage(request.get()))
            {
                return XCode::CallArgsError;
            }
            message.Body()->clear();
            if (this->mHasUserId)
            {
                long long userId = 0;
                if (!message.GetHead().Get(this->mKey, userId))
                {
                    return XCode::CallArgsError;
                }
                int code = (_o->*_objfunc)(userId, *request, *response);
                if (code == XCode::Ok)
                {
                    if (!message.WriteMessage(response.get()))
                    {
                        return XCode::SerializationFailure;
                    }
                    return XCode::Ok;
                }
                return code;
            }
			int code = (_o->*_func)(*request, *response);
			if (code == XCode::Ok)
			{
                if(!message.WriteMessage(response.get()))
                {
                    return XCode::SerializationFailure;
                }
                return XCode::Ok;
			}
			return code;
		}
	 private:
		T* _o;
		const bool mHasUserId;
		const std::string mKey;
		ServiceMethodType3<T, T1, T2> _func;
		ServiceMethodType33<T, T1, T2> _objfunc;
	};

	template<typename T, typename T1>
	class ServiceMethod4 final : public ServiceMethod
	{
	 public:
		ServiceMethod4(const std::string name, T* o, ServiceMethodType4<T, T1> func, std::string key)
			: ServiceMethod(name), _o(o),mHasUserId(false), mKey(std::move(key)), _func(func)
		{
		}

		ServiceMethod4(const std::string name, T* o, ServiceMethodType44<T, T1> func, std::string key)
			: ServiceMethod(name), _o(o), mHasUserId(true), mKey(std::move(key)), _objfunc(func)
		{
		}
	 public:
		int Invoke(rpc::Message & message) override
		{
			std::unique_ptr<T1> response = std::make_unique<T1>();
            if (this->mHasUserId)
			{
                long long userId = 0;
                if(!message.GetHead().Get(this->mKey, userId))
                {
                    return XCode::CallArgsError;
                }
				int code = (_o->*_objfunc)(userId, *response);
				if(code == XCode::Ok)
				{
					message.WriteMessage(response.get());
					return XCode::Ok;
				}
				return code;
			}
			int code = (_o->*_func)(*response);
			if(code == XCode::Ok)
			{
				message.WriteMessage(response.get());
				return XCode::Ok;
			}
			return code;
		}
	 private:
		T* _o;
		const bool mHasUserId;
		const std::string mKey;
		ServiceMethodType4<T, T1> _func;
		ServiceMethodType44<T, T1> _objfunc;
	};

	template<typename T>
	class ServiceMethod6 final : public ServiceMethod
	{
	 public:
		ServiceMethod6(const std::string name, T* o, ServiceMethodType6<T> func, std::string key)
			: ServiceMethod(name), _o(o), mKey(std::move(key)), _func(func)
		{

		}

	 public:
		int Invoke(rpc::Message & message) override
		{
			return (_o->*_func)(message);
		}
	 private:
		T* _o;
		const std::string mKey;
		ServiceMethodType6<T> _func;
	};

	template<typename T>
	class ServiceMethod7 final : public ServiceMethod
	{
	public:
		ServiceMethod7(const std::string name, T* o, ServiceMethodType7<T> func, std::string key)
			: ServiceMethod(name), _o(o), mKey(std::move(key)), _func(func)
		{

		}

	public:
		int Invoke(rpc::Message& message) override
		{
			return (_o->*_func)(message, message);
		}
	private:
		T* _o;
		std::string mKey;
		ServiceMethodType7<T> _func;
	};

	template<typename T>
	class ServiceMethod8 final : public ServiceMethod
	{
	public:
		ServiceMethod8(const std::string name, T* o, ServiceMethodType8<T> func, std::string key)
				: ServiceMethod(name), _o(o), mKey(std::move(key)), _func(func)
		{

		}

	public:
		int Invoke(rpc::Message& message) override
		{
			std::unique_ptr<json::r::Document> request = std::make_unique<json::r::Document>();
			if(!request->Decode(message.GetBody()))
			{
				return XCode::ParseJsonFailure;
			}
			return (_o->*_func)(*request);
		}
	private:
		T* _o;
		std::string mKey;
		ServiceMethodType8<T> _func;
	};

	template<typename T>
	class ServiceMethod9 final : public ServiceMethod
	{
	public:
		ServiceMethod9(const std::string name, T* o, ServiceMethodType9<T> func, std::string key)
				: ServiceMethod(name), _o(o), mKey(std::move(key)), _func(func)
		{

		}

	public:
		int Invoke(rpc::Message& message) override
		{
			std::unique_ptr<json::r::Document> request = std::make_unique<json::r::Document>();
			std::unique_ptr<json::w::Document> response = std::make_unique<json::w::Document>();
			if(!request->Decode(message.GetBody()))
			{
				return XCode::ParseJsonFailure;
			}
			int code = (_o->*_func)(*request, *response);
			message.WriteMessage(response.get());
			return code;
		}
	private:
		T* _o;
		std::string mKey;
		ServiceMethodType9<T> _func;
	};

	template<typename T, typename T1>
	class ServiceMethod34 final : public ServiceMethod
	{
	public:
		ServiceMethod34(const std::string name, T* o, ServiceMethodType34<T, T1> func, std::string key)
				: ServiceMethod(name), _o(o), mKey(std::move(key)), _func(func)
		{

		}

	public:
		int Invoke(rpc::Message& message) override
		{
			std::unique_ptr<T1> request = std::make_unique<T1>();
			if(!message.ParseMessage(request.get()))
			{
				return XCode::CallArgsError;
			}
			message.Body()->clear();
			return (_o->*_func)(*request, message);
		}
	private:
		T* _o;
		std::string mKey;
		ServiceMethodType34<T, T1> _func;
	};
}