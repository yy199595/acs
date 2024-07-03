//
// Created by leyi on 2023/11/3.
//

#ifndef APP_ZIP_H
#define APP_ZIP_H
#include<string>
namespace help
{
	namespace zip
	{
		std::string Compress(const std::string & input);
		bool CompressFile(const std::string & path, std::string & output);
		bool CompressData(const std::string & input, std::string & output);
		bool CompressData(const char * input, size_t size, std::string & output);
	}
}

#endif //APP_ZIP_H
