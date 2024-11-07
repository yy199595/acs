#pragma once

#include <string>
#include <vector>
#include <fstream>
namespace help
{
    namespace fs
    {
		constexpr size_t KB = 1024;
		constexpr size_t MB = 1024 * 1024;
		constexpr size_t GB = 1024 * 1024 * 1024;
		
		extern bool FileIsExist(const std::string &path);
		extern bool GetFileSize(std::fstream & fs, size_t & size);
		extern long long GetLastWriteTime(const std::string & path);
		extern bool GetFileLine(const std::string & path, size_t & size);
		extern bool GetFileSize(const std::string & path, size_t & size);
		extern bool GetFileSize(const std::string & path, std::string & size);
		extern bool GetFileName(const std::string& path, std::string& name);
		extern bool GetFileType(const std::string & path, std::string & type);
        extern bool ReadTxtFile(const std::string &path, std::string &outFile);
		extern bool ChangeName(const std::string & path, const std::string & name);
		extern bool WriterFile(const std::string &path, const std::string &fileContent);
		extern bool ReadTxtFile(const std::string &path, std::vector<std::string> &outLines);
	}

}// namespace FileHelper
