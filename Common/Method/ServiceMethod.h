#pragma once
#include<Other/ObjectFactory.h>
#include<Message/com.pb.h>
#include<XCode/XCode.h>
#include<google/protobuf/any.h>
#include<google/protobuf/any.pb.h>
#include"Json/JsonReader.h"
#include"google/protobuf/util/json_util.h"
using namespace google::protobuf;
namespace Sentry
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

	template<typename T, typename T1>
	using ServiceMethodType5 = XCode(T::*)(const std::string & address, const T1 &);

	template<typename T>
	using ServiceMethodType6 = XCode(T::*)(const std::string & address);


//	template<typename T, typename T1, typename T2>
//	using ServiceMethodType55 = XCode(T::*)(const std::string & address, const T1 &, T2 &);

}// namespace Sentry


namespace Sentry
{

	class ServiceMethod
	{
	 public:
		explicit ServiceMethod(std::string name)
			: mName(std::move(name)) {}
	 public:
		virtual bool IsLuaMethod() = 0;
		virtual XCode Invoke(const com::rpc::request& request, com::rpc::response& response) = 0;
		const std::string& GetName()
		{
			return this->mName;
		}

    protected:
        template<typename T1>
        std::shared_ptr<T1> GetRequest(const com::rpc::request& request)
        {
            if(request.type() == com::rpc_msg_type_json)
            {
                if(request.json().empty())
                {
                    return nullptr;
                }
                std::shared_ptr<T1> requestData(new T1());
                const std::string & json = request.json();
                if(util::JsonStringToMessage(json, requestData.get()).ok())
                {
                    return requestData;
                }
            }
            else if(request.type() == com::rpc_msg_type_proto)
            {
                if(!request.has_data())
                {
                    return nullptr;
                }
                std::shared_ptr<T1> requestData(new T1());
                if(request.data().UnpackTo(requestData.get()))
                {
                    return requestData;
                }
            }
            return nullptr;
        }
	 private:
		std::string mName;
	};

	template<typename T>
	class ServiceMethod1 : public ServiceMethod
	{
	 public:
		ServiceMethod1(const std::string name, T* o, ServiceMethodType1<T> func)
			: ServiceMethod(name), _o(o), _func(func), mHasUserId(false)
		{
		}

		ServiceMethod1(const std::string name, T* o, ServiceMethodType11<T> func)
			: ServiceMethod(name), _o(o), _objfunc(func), mHasUserId(true)
		{
		}
	 public:

		XCode Invoke(const com::rpc::request& request, com::rpc::response& response) override
		{
			if (!this->mHasUserId)
			{
				return (_o->*_func)();
			}
			return (_o->*_objfunc)(request.user_id());
		}

		bool IsLuaMethod() override
		{
			return false;
		};
	 private:
		T* _o;
		bool mHasUserId;
		ServiceMethodType1<T> _func;
		ServiceMethodType11<T> _objfunc;
	};
	template<typename T, typename T1>
	class ServiceMethod2 : public ServiceMethod
	{
	 public:
		ServiceMethod2(const std::string name, T* o, ServiceMethodType2<T, T1> func)
			: ServiceMethod(name), _o(o), _func(func), mHasUserId(false)
		{
		}

		ServiceMethod2(const std::string name, T* o, ServiceMethodType22<T, T1> func)
			: ServiceMethod(name), _o(o), _objfunc(func), mHasUserId(true)
		{
		}
	 public:
		XCode Invoke(const com::rpc::request& request, com::rpc::response& response) override
		{
            std::shared_ptr<T1> requestData = this->GetRequest<T1>(request);
            if(requestData == nullptr)
            {
                return XCode::CallArgsError;
            }
			if (!this->mHasUserId)
			{
				return (_o->*_func)(*requestData);
			}
			return (_o->*_objfunc)(request.user_id(), *requestData);
		}
		bool IsLuaMethod() override
		{
			return false;
		};
	 private:
		T* _o;
		bool mHasUserId;
		ServiceMethodType2<T, T1> _func;
		ServiceMethodType22<T, T1> _objfunc;
	};

	template<typename T, typename T1, typename T2>
	class ServiceMethod3 : public ServiceMethod
	{
	 public:
		typedef XCode(T::*ServerFunc)(long long, const T1&, T2&);
		ServiceMethod3(const std::string name, T* o, ServiceMethodType3<T, T1, T2> func)
			: ServiceMethod(name), _o(o), _func(func), mHasUserId(false)
		{
		}

		ServiceMethod3(const std::string name, T* o, ServiceMethodType33<T, T1, T2> func)
			: ServiceMethod(name), _o(o), _objfunc(func), mHasUserId(true)
		{
		}
	 public:
		XCode Invoke(const com::rpc::request& request, com::rpc::response& response) override
		{
            std::shared_ptr<T2> responseData(new T2());
            std::shared_ptr<T1> requestData = this->GetRequest<T1>(request);
			if (this->mHasUserId)
			{
				XCode code = (_o->*_objfunc)(request.user_id(), *requestData, *responseData);
				if (code == XCode::Successful)
				{
                    if(request.type() == com::rpc_msg_type_json &&
                            util::MessageToJsonString(*responseData, response.mutable_json()).ok())
                    {
                        return XCode::Successful;
                    }
                    if(request.type() == com::rpc_msg_type_proto)
                    {
                        response.mutable_data()->PackFrom(*responseData);
                        return XCode::Successful;
                    }
                    return XCode::SerializationFailure;
				}
				return code;
			}
			XCode code = (_o->*_func)(*requestData, *responseData);
			if (code == XCode::Successful)
			{
                if(request.type() == com::rpc_msg_type_json &&
                   util::MessageToJsonString(*responseData, response.mutable_json()).ok())
                {
                    return XCode::Successful;
                }
                if(request.type() == com::rpc_msg_type_proto)
                {
                    response.mutable_data()->PackFrom(*responseData);
                    return XCode::Successful;
                }
                return XCode::SerializationFailure;
			}
			return code;
		}
		bool IsLuaMethod() override
		{
			return false;
		};
	 private:
		T* _o;
		bool mHasUserId;
		ServiceMethodType3<T, T1, T2> _func;
		ServiceMethodType33<T, T1, T2> _objfunc;
	};

	template<typename T, typename T1>
	class ServiceMethod4 : public ServiceMethod
	{
	 public:
		ServiceMethod4(const std::string name, T* o, ServiceMethodType4<T, T1> func)
			: ServiceMethod(name), _o(o), _func(func), mHasUserId(false)
		{
		}

		ServiceMethod4(const std::string name, T* o, ServiceMethodType44<T, T1> func)
			: ServiceMethod(name), _o(o), _objfunc(func), mHasUserId(true)
		{
		}
	 public:
		XCode Invoke(const com::rpc::request& request, com::rpc::response& response) override
		{
			std::shared_ptr<T1> responseData(new T1());
			if (this->mHasUserId)
			{
				XCode code = (_o->*_objfunc)(request.user_id(), *responseData);
				if (code == XCode::Successful)
				{
                    if(request.type() == com::rpc_msg_type_json &&
                       util::MessageToJsonString(*responseData, response.mutable_json()).ok())
                    {
                        return XCode::Successful;
                    }
                    if(request.type() == com::rpc_msg_type_proto)
                    {
                        response.mutable_data()->PackFrom(*responseData);
                        return XCode::Successful;
                    }
                    return XCode::SerializationFailure;
				}
				return code;
			}
			XCode code = (_o->*_func)(*responseData);
			if (code == XCode::Successful)
			{
                if(request.type() == com::rpc_msg_type_json &&
                   util::MessageToJsonString(*responseData, response.mutable_json()).ok())
                {
                    return XCode::Successful;
                }
                if(request.type() == com::rpc_msg_type_proto)
                {
                    response.mutable_data()->PackFrom(*responseData);
                    return XCode::Successful;
                }
                return XCode::SerializationFailure;
			}
			return code;
		}
		bool IsLuaMethod() override
		{
			return false;
		};
	 private:
		T* _o;
		bool mHasUserId;
		ServiceMethodType4<T, T1> _func;
		ServiceMethodType44<T, T1> _objfunc;
	};


	template<typename T, typename T1>
	class ServiceMethod5 : public ServiceMethod
	{
	public:
		ServiceMethod5(const std::string name, T* o, ServiceMethodType5<T, T1> func)
				: ServiceMethod(name), _o(o), _func(func)
		{

		}

	public:
		bool IsLuaMethod() override
		{
			return false;
		};
		XCode Invoke(const com::rpc::request& request, com::rpc::response& response) override
		{
			std::shared_ptr<T1> requestData = this->GetRequest<T1>(request);
			return (_o->*_func)(request.address(), *requestData);
		}

	private:
		T* _o;
		ServiceMethodType5<T, T1> _func;
	};

	template<typename T>
	class ServiceMethod6 : public ServiceMethod
	{
	 public:
		ServiceMethod6(const std::string name, T* o, ServiceMethodType6<T> func)
			: ServiceMethod(name), _o(o), _func(func)
		{

		}

	 public:
		bool IsLuaMethod() override { return false; };
		XCode Invoke(const com::rpc::request& request, com::rpc::response& response) override
		{
			assert(!request.address().empty());
			return (_o->*_func)(request.address());
		}
	 private:
		T* _o;
		ServiceMethodType6<T> _func;
	};
}