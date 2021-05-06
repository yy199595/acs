#pragma once
#include<map>
#include<string>
#include<memory>
#include<vector>
#include<functional>
#include<type_traits>
#include<unordered_map>
#include<google/protobuf/message.h>
// 普通数据类型
namespace CommonParameter
{
	template<typename T>
	struct IsCommonParameter { constexpr static  bool value = false; };

	/*template<typename T>
	struct IsCommonParameter<const T> { constexpr static  bool value = false; };*/

	template<> struct IsCommonParameter<int> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<unsigned int> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<long long> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<float> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<double> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<unsigned long long> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<const unsigned long long> { constexpr static  bool value = true; };


	template<> struct IsCommonParameter<short> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<unsigned short> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<bool> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<std::string> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<const std::string> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<std::string &> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<const std::string &> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<const char *> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<char *> { constexpr static  bool value = true; };

	template<> struct IsCommonParameter<void> { constexpr static  bool value = true; };

}
// std容器类型
namespace ContainerParameter
{
	template<typename T>
	struct IsContainerParameter { constexpr static bool value = false; };

	template<typename T>
	struct IsContainerParameter<const T> { constexpr static bool value = false; };

	template<typename T>
	struct IsContainerParameter<std::vector<T>> { constexpr static bool value = true; };

	template<typename T>
	struct IsContainerParameter<std::vector<T> &> { constexpr static bool value = true; };

	template<typename T>
	struct IsContainerParameter<std::vector<T> *> { constexpr static bool value = true; };

	template<typename T>
	struct IsContainerParameter<std::shared_ptr<std::vector<T>>> { constexpr static bool value = true; };

	template<typename Key, typename Value>
	struct IsContainerParameter<std::map<Key, Value>> { constexpr static bool value = true; };

	template<typename Key, typename Value>
	struct IsContainerParameter<std::map<Key, Value> &> { constexpr static bool value = true; };

	template<typename Key, typename Value>
	struct IsContainerParameter<std::map<Key, Value> *> { constexpr static bool value = true; };

	template<typename Key, typename Value>
	struct IsContainerParameter<std::unordered_map<Key, Value>> { constexpr static bool value = true; };

	template<typename Key, typename Value>
	struct IsContainerParameter<std::unordered_map<Key, Value> &> { constexpr static bool value = true; };

	template<typename Key, typename Value>
	struct IsContainerParameter<std::unordered_map<Key, Value> *> { constexpr static bool value = true; };
}
// 函数类型
namespace FunctionParameter
{
	template<typename T>
	struct IsFunctionParameter { constexpr static bool value = false; };

	template<typename Ret, typename ...Args>
	struct IsFunctionParameter<std::function<Ret(Args ...)>> { constexpr static bool value = true; };

}

namespace TableParameter
{

}

namespace ConstParameter
{
	template<typename T>
	struct ConstProxy { using Type = T; constexpr static  bool value = false; };

	template<typename T>
	struct ConstProxy<const T> { using Type = T; constexpr static  bool value = true; };

	template<typename T>
	struct IsConstParameter { constexpr static bool value = false; };

	template<typename T>
	struct IsConstParameter<const T> { constexpr static bool value = true; };

	template<typename T>
	struct IsConstParameter<const T *> { constexpr static bool value = true; };

	template<typename T>
	struct IsConstParameter<const T &> { constexpr static bool value = true; };
}

namespace ProtocParameter
{
	template<typename T>
	struct IsProtocParameter { constexpr static bool value = std::is_base_of<google::protobuf::Message, T>::value; };

	template<typename T>
	struct IsProtocParameter<const T> { constexpr static bool value = false; };

	template<typename T>
	struct IsProtocParameter<T * > { constexpr static bool value = std::is_base_of<google::protobuf::Message, T>::value; };

	template<typename T>
	struct IsProtocParameter<T&> { constexpr static bool value = std::is_base_of<google::protobuf::Message, T>::value; };

	template<typename T>
	struct IsProtocParameter<std::shared_ptr<T>> { constexpr static bool value = std::is_base_of<google::protobuf::Message, T>::value; };
}

// 自定义数据类型
namespace UserDataParameter
{
	template<typename T>
	struct IsUserDataParameter
	{
		static constexpr bool value =
			!FunctionParameter::IsFunctionParameter<T>::value &&
			!ContainerParameter::IsContainerParameter<T>::value &&
			!CommonParameter::IsCommonParameter<T>::value &&
			!ProtocParameter::IsProtocParameter<T>::value &&
			!ConstParameter::IsConstParameter<T>::value &&
			!std::is_enum<T>::value;

	};
}

