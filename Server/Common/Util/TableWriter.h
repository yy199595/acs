#pragma once
#include<string>
#include<fstream>
namespace SoEasy
{
	class TableWriter
	{
	public:
		TableWriter(const std::string & path) {}
	private:
		const std::string mPath;
	};
}