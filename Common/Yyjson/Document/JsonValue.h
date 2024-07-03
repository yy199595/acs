//
// Created by yy on 2024/6/29.
//

#ifndef APP_JSONVALUE_H
#define APP_JSONVALUE_H
#include <string>
namespace json
{
	enum class Type
	{
		None,
		Int,
		String,
		Float32
	};

	struct Value
	{
	public:
		Value(long value) : mType(Type::Int), mValue(std::to_string(value)) { }
		Value(float value) : mType(Type::Float32), mValue(std::to_string(value)) { }
		Value(const std::string& value) : mType(Type::String), mValue(value) { }
	public:
		const Type mType;
		const std::string mValue;
	};
}

#endif //APP_JSONVALUE_H
