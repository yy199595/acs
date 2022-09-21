#pragma once
#include <string>
template<typename T>
struct TypeReflection
{
public:
	static constexpr bool Value = false;
	static constexpr char * Name = nullptr;
};

namespace Sentry
{
	template<typename T>
	inline bool GetTypeName(std::string& name)
	{
		if (TypeReflection<T>::Value)
		{
			name.assign(TypeReflection<T>::Name);
			return true;
		}
		return false;
	}
}