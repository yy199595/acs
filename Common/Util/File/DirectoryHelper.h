#pragma once

#include <string>
#include <vector>

namespace help
{
	namespace dir
	{
		extern bool IsDir(const std::string& str);
		extern bool MakeDir(const std::string& dir);
		extern bool DeleteDir(const std::string& dir);
		extern int RemoveAllFile(const std::string& dir);
		extern bool IsValidPath(const std::string& path);
		extern bool DirectorIsExist(const std::string& dir);
		extern bool GetDirByPath(const std::string& path, std::string& director);
		extern bool GetFileName(const std::string& fullName, std::string& fileName);
		extern bool GetFilePaths(const std::string& dir, std::vector<std::string>& paths, bool r = true);
		extern bool GetDirAndFileName(const std::string& path, std::string& director, std::string& fileName);
		extern bool GetFilePaths(const std::string& dir, const std::string& format, std::vector<std::string>& paths);
		extern bool GetDirAndFiles(const std::string& dir, std::vector<std::string>& dirs, std::vector<std::string>& files);
	}
}
