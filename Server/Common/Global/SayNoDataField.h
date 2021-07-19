#pragma once
#include<Define/CommonDef.h>
namespace Sentry
{
	class SayNoDataField
	{
	public:
		SayNoDataField(const std::string & str) : data(str) { }
	public:
		template<typename T> T AsNumber() { return T(); }
		template<>  int AsNumber() { return std::stoi(data); }
		template<>  float AsNumber() { return std::stof(data); }
		template<>  double AsNumber() { return std::stod(data); }
		template<>  long long AsNumber() { return std::stoll(data); }
		template<>  unsigned int AsNumber() { return std::stoul(data); }
	public:

	public:
		const std::string & GetString() { return data; }
	private:
		const std::string & data;
	};
}