//
// Created by yjz on 2022/1/17.
//

#ifndef GAMEKEEPER_SUBMETHOD_H
#define GAMEKEEPER_SUBMETHOD_H
#include<iostream>
#include"XCode/XCode.h"
#include"Protocol/com.pb.h"
#include"Define/CommonLogDef.h"
#include<google/protobuf/message.h>

namespace Sentry
{
	template<typename T, typename T1>
	using SubFunction = XCode(T::*)(const T1 &);

	template<typename T, typename T1, typename T2>
	using SubFunction2 = XCode(T::*)(const T1 &, T2 &);

	class SubMethod
	{
	 public:
		virtual XCode OnPublish(const com::Rpc::Request & request, com::Rpc::Response & response) = 0;
	};

	template<typename T, typename T1>
	class JsonSubMethod : public SubMethod
	{
	 public:
		JsonSubMethod(T* obj, SubFunction<T, T1> func)
			: mObj(obj), mFunction(func) { }
	 public:
		XCode OnPublish(const com::Rpc::Request &request, com::Rpc::Response &response) final
		{
			if(!request.has_data() || !request.data().Is<T1>() )
			{
				return XCode::CallArgsError;
			}
			std::shared_ptr<T1> data(new T1());
			if(!request.data().UnpackTo(data.get()))
			{
				return XCode::CallArgsError;
			}
			return (this->mObj->*mFunction)(*data);
		}
	 private:
		T* mObj;
		SubFunction<T, T1> mFunction;
	};

	template<typename T, typename T1, typename T2>
	class JsonSubMethod2 : public SubMethod
	{
	public:
		JsonSubMethod2(T* obj, SubFunction2<T, T1, T2> func)
				: mObj(obj), mFunction(func) { }
	public:
		XCode OnPublish(const com::Rpc::Request &request, com::Rpc::Response &response) final
		{
			if(!request.has_data() || !request.data().Is<T1>() )
			{
				return XCode::CallArgsError;
			}
			std::shared_ptr<T1> data(new T1());
			if(!request.data().UnpackTo(data.get()))
			{
				return XCode::CallArgsError;
			}
			std::shared_ptr<T2> data1(new T2());
			XCode code = (this->mObj->*mFunction)(*data, *data1);
			if(code == XCode::Successful)
			{
				response.mutable_data()->PackFrom(*data1);
				return XCode::Successful;
			}
			return code;
		}
	private:
		T* mObj;
		SubFunction2<T, T1, T2> mFunction;
	};
}
#endif //GAMEKEEPER_SUBMETHOD_H
