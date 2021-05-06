#pragma once
#include<string>
#include<vector>
namespace DirectoryHelper
{
	extern bool MakeDir(const std::string dir);
	extern bool DeleteDir(const std::string dir);
	extern bool DirectorIsExist(const std::string dir);
	extern bool GetFilePaths(const std::string dir, std::vector<std::string> & paths);
	extern bool GetFilePaths(const std::string dir, std::string format, std::vector<std::string> & paths);
	extern bool GetDirAndFileName(const std::string path, std::string & director, std::string & fileName);
}