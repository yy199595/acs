//
// Created by mac on 2022/4/12.
//

#ifndef SERVER_METHODREGISTER_H
#define SERVER_METHODREGISTER_H

#include"SubMethod.h"
#include"ServiceMethod.h"
#include"HttpServiceMethod.h"
#include"Script/LuaInclude.h"

namespace Sentry
{
	class ServiceMethodRegister
	{
	public:
		ServiceMethodRegister(const std::string & name, void * o);
	public:
		template<typename T>
		bool Bind(std::string name, ServiceMethodType1<T> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod1<T>>(name, (T*)this->mObj, func));
		}

		template<typename T>
		bool Bind(std::string name, ServiceMethodType11<T> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod1<T>>(name, (T*)this->mObj, func));
		}

		template<typename T, typename T1>
		inline typename std::enable_if<std::is_base_of<Message, T1>::value, bool>::type
		Bind(std::string name, ServiceMethodType2<T, T1> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this->mObj, func));
		}

		template<typename T, typename T1>
		inline typename std::enable_if<std::is_base_of<Message, T1>::value, bool>::type
		Bind(std::string name, ServiceMethodType22<T, T1> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this->mObj, func));
		}

		template<typename T, typename T1, typename T2>
		inline typename std::enable_if<std::is_base_of<Message, T1>::value
			&& std::is_base_of<Message, T1>::value, bool>::type
		Bind(std::string name, ServiceMethodType3<T, T1, T2> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this->mObj, func));
		}

		template<typename T, typename T1, typename T2>
		inline typename std::enable_if<std::is_base_of<Message, T1>::value
			&& std::is_base_of<Message, T1>::value, bool>::type
		Bind(std::string name, ServiceMethodType33<T, T1, T2> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this->mObj, func));
		}

		template<typename T, typename T1>
		inline typename std::enable_if<std::is_base_of<Message, T1>::value, bool>::type
		Bind(std::string name, ServiceMethodType4<T, T1> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this->mObj, func));
		}

		template<typename T, typename T1>
		inline typename std::enable_if<std::is_base_of<Message, T1>::value, bool>::type
		Bind(std::string name, ServiceMethodType44<T, T1> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this->mObj, func));
		}

		template<typename T, typename T1>
		bool BindAddress(std::string name, ServiceMethodType5<T, T1> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod5<T, T1>>(name, (T*)this->mObj, func));
		}

		template<typename T>
		bool BindAddress(std::string name, ServiceMethodType6<T> func)
		{
			return this->AddMethod(std::make_shared<ServiceMethod6<T>>(name, (T*)this->mObj, func));
		}

	public:
		bool LoadLuaMethod(lua_State *lua);
		bool AddMethod(std::shared_ptr<ServiceMethod> method);
		std::shared_ptr<ServiceMethod> GetMethod(const std::string& name);
	private:
		void * mObj;
		const std::string mService;
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mMethodMap;
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mLuaMethodMap;

	};

//	template<typename T, typename T1, typename T2>
//	inline typename std::enable_if<std::is_same<T1, std::string>::value && std::is_base_of<Message, T2>::value, bool>::type
//	ServiceMethodRegister::Bind(std::string name, ServiceMethodType3<T, T1, T2> func)
//	{
//		return this->AddMethod(std::make_shared<ServiceMethod5<T, T1>>(name, (T*)this->mObj, func));
//	}
}


namespace Sentry
{
	class ServiceEventRegister
	{
	public:
		ServiceEventRegister(const std::string& name, void* o)
			: mObj(o), mService(name) {}
	public:
		template<typename T>
		bool Sub(const std::string & id, EventMethodType1<T> func)
		{
			auto iter = this->mEventMethodMap.find(id);
			if(iter != this->mEventMethodMap.end())
			{
				return false;
			}
			this->mEventMethodMap.emplace(id, std::make_shared<EventMethod1<T>>(id, (T*)this->mObj, func));
			return true;
		}

		template<typename T>
		bool Sub(const std::string & id, EventMethodType2<T> func)
		{
			auto iter = this->mEventMethodMap.find(id);
			if(iter != this->mEventMethodMap.end())
			{
				return false;
			}
			this->mEventMethodMap.emplace(id, std::make_shared<EventMethod2<T>>(id, (T*)this->mObj, func));
			return true;
		}
		std::shared_ptr<EventMethod> GetEvent(const std::string & eveId);
		const size_t GetEventSize() { return this->mEventMethodMap.size();}
	private:
		void * mObj;
		const std::string mService;
		std::unordered_map<std::string, std::shared_ptr<EventMethod>> mEventMethodMap;
	};
}

namespace Sentry
{
	class HttpServiceRegister
	{
	 public:
		HttpServiceRegister(void * o) : mObj(o) { }
		std::shared_ptr<HttpServiceMethod> GetMethod(const std::string & name);
	 public:
		template<typename T>
		bool Bind(const std::string& name, HttpJsonMethod1<T> func)
		{
			auto iter = this->mHttpMethodMap.find(name);
			if (iter != this->mHttpMethodMap.end())
			{
				return false;
			}
			this->mHttpMethodMap.emplace(name, std::make_shared<
				HttpServiceJsonMethod1<T>>((T*)this->mObj, std::move(func)));
			return true;
		}

		template<typename T>
		bool Bind(const std::string& name, HttpJsonMethod2<T> func)
		{
			auto iter = this->mHttpMethodMap.find(name);
			if (iter != this->mHttpMethodMap.end())
			{
				return false;
			}
			this->mHttpMethodMap.emplace(name, std::make_shared<
				HttpServiceJsonMethod2<T>>((T*)this->mObj, std::move(func)));
			return true;
		}

	 private:
		void * mObj;
		std::unordered_map<std::string, std::shared_ptr<HttpServiceMethod>> mHttpMethodMap;
	};
}



#endif //SERVER_METHODREGISTER_H
