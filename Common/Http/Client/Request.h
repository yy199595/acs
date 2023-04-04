//
// Created by yjz on 2022/10/24.
//

#ifndef _REQUEST_H_
#define _REQUEST_H_
#include<string>
#include<unordered_map>
namespace Tendo
{
	class HttpRequest
	{
	 public:
		HttpRequest(const std::string& method)
			: mMethod(method)
		{
		}
	 public:
		virtual const std::string& Path() const = 0;
	 public:
		bool AddHead(const std::string& k, int v);
		bool AddHead(const std::string& k, const std::string& v);
		bool GetHead(const std::string& k, std::string& value) const;
		const std::string& Method() const { return this->mMethod; };
	 private:
		std::string mMethod;
		std::unordered_map<std::string, std::string> mHead;
	};
}

#endif //_REQUEST_H_
