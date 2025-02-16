#include "DirectoryHelper.h"

#ifdef __OS_WIN__
#include <codecvt>
#include <direct.h>
#include <io.h>
#include <windows.h>

#else
#include <dirent.h>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#endif
#ifdef __OS_WIN__
#include <codecvt>
#define MakeDirectory(path) _mkdir(path.c_str())
#else
#define MakeDirectory(path) mkdir((path).c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH)
#endif
#define PATH_MAX_LENGHT 1024

#include<regex>
#include <bundled/format.h>

namespace help
{
	bool dir::IsDir(const std::string& str)
	{
#ifdef __OS_WIN__
		DWORD fileAttributes = GetFileAttributes(str.c_str());
		if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
			return false;
		}
		if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			   return true;
		}
		return false;
#else
		struct stat statBuf;

		if (stat(str.c_str(), &statBuf) != 0)
		{
			return false;
		}

		if (S_ISREG(statBuf.st_mode))
		{
			return false;
		}
		else if (S_ISDIR(statBuf.st_mode))
		{
			return true;
		}
		return false;
#endif
	}

	bool dir::MakeDir(const std::string& dir)
	{
		if (dir::DirectorIsExist(dir))
		{
			return true;
		}

		for (size_t index = 0; index < dir.size(); index++)
		{
			if (dir[index] == '/' || dir[index] == '\\')
			{
				const std::string path = dir.substr(0, index);
#ifdef __OS_WIN__
				std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
				std::wstring newPath = converter.from_bytes(path);
				_wmkdir(newPath.c_str());
#else
				MakeDirectory(path);
#endif
			}
		}
#ifdef __OS_WIN__
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring newPath = converter.from_bytes(dir);
		return _wmkdir(newPath.c_str()) != -1;
#else
		return MakeDirectory(dir) != -1;
#endif
	}

	bool dir::IsValidPath(const std::string& path)
	{
		const std::regex pathRegex(R"(^([a-zA-Z]:)?[\\/](?:[^\\/:\*\?"<>\|]+[\\/])*[^\\/:\*\?"<>\|]*$)");
		return std::regex_match(path, pathRegex);
	}

	bool dir::DeleteDir(const std::string& dir)
	{
#ifdef __OS_WIN__
		return _rmdir(dir.c_str()) != -1;
#else
		return rmdir(dir.c_str()) != -1;
#endif
	}

	int dir::RemoveAllFile(const std::string& dir)
	{
		int count = 0;
		std::vector<std::string> filePaths;
		dir::GetFilePaths(dir, filePaths);
		for(const std::string & path : filePaths)
		{
			if(std::remove(path.c_str()) == 0)
			{
				count++;
			}
		}
		dir::DeleteDir(dir);
		return count;
	}

	bool dir::DirectorIsExist(const std::string& dir)
	{
#ifdef __OS_WIN__
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring newDir = converter.from_bytes(dir);
		//return _access(dir.c_str(), 0) == 0;
		return  _waccess(newDir.c_str(), 0) == 0;
#else
		return access(dir.c_str(), F_OK) == 0;
#endif
	}

	bool dir::GetDirAndFiles(const std::string& path,
			std::vector<std::string>& directorys, std::vector<std::string>& files)
	{
#ifdef __OS_WIN__
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring temPath = converter.from_bytes(path) + L"/*.*";
		WIN32_FIND_DATAW findFileData;
		HANDLE fileHandle = FindFirstFileW(temPath.c_str(), &findFileData);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		while (true)
		{
			if (findFileData.cFileName[0] != '.')
			{
				std::string name = converter.to_bytes(findFileData.cFileName);
				if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					std::string newPath = fmt::format("{}/{}", path, name);
					directorys.emplace_back(newPath);
				}
				else
				{
					std::string newPath = fmt::format("{}/{}", path, name);
					files.emplace_back(newPath);
				}
			}
			//如果是当前路径或者父目录的快捷方式，或者是普通目录，则寻找下一个目录或者文件
			if (!FindNextFileW(fileHandle, &findFileData))
			{
				break;
			}
		}
		FindClose(fileHandle);
#else
		DIR* dir = opendir(path.c_str());
		if (dir == NULL)
		{
			return false;
		}
		while (true)
		{
			struct dirent* ptr = readdir(dir);
			if (ptr == NULL)
			{
				break;
			}
			if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
			{
				continue;
			}
			else if (ptr->d_type == DT_DIR)
			{
				directorys.emplace_back(path + "/" + ptr->d_name);
			}
			else if (ptr->d_type == 8)
			{
				files.emplace_back(path + "/" + ptr->d_name);
			}
		}
		return true;
#endif
	}

	bool dir::GetFilePaths(const std::string& path, std::vector<std::string>& paths, bool r)
	{
#ifdef __OS_WIN__
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring temPath = converter.from_bytes(path) + L"/*.*";

		WIN32_FIND_DATAW findFileData;
		HANDLE fileHandle = FindFirstFileW(temPath.c_str(), &findFileData);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		while (true)
		{
			if (findFileData.cFileName[0] != '.')
			{
				std::string name = converter.to_bytes(findFileData.cFileName);
				if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					std::string newDir = fmt::format("{}/{}", path, name);
					GetFilePaths(newDir, paths);
				}
				else
				{
					std::string newPath = fmt::format("{}/{}", path, name);
					paths.emplace_back(newPath);
				}
			}
			//如果是当前路径或者父目录的快捷方式，或者是普通目录，则寻找下一个目录或者文件
			if (!FindNextFileW(fileHandle, &findFileData))
			{
				break;
			}
		}
		FindClose(fileHandle);
		return true;
#else
		DIR *dir = opendir(path.c_str());
		if (dir == NULL)
		{
			return false;
		}

		while (true)
		{
			struct dirent *ptr = readdir(dir);
			if (ptr == NULL)
			{
				break;
			}
			if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
			{
				continue;
			}
			else if (ptr->d_type == DT_DIR && r)
			{
				std::string strFile = path + "/" + ptr->d_name;
				GetFilePaths(strFile, paths);
			}
			else if (ptr->d_type == 8)///file
			{
				std::string strFile = path + "/" + ptr->d_name;
				paths.push_back(strFile);
			}
		}
		return closedir(dir) != -1;
#endif
	}

	bool dir::GetFilePaths(const std::string& path, std::string format, std::vector<std::string>& paths)
	{
		std::vector<std::string> allPaths;
		if (GetFilePaths(path, allPaths))
		{
			if (format[0] == '*')
			{ format = format.substr(1); }
			for (size_t index = 0; index < allPaths.size(); index++)
			{
				if (allPaths[index].find(format) != std::string::npos)
				{
					paths.push_back(allPaths[index]);
				}
			}
			return true;
		}
		return false;
	}

	bool dir::GetDirByPath(const std::string& path, std::string& director)
	{
		/*std::string pattern("^[a-zA-Z]:|([\\\\/]|[^\\s\\\\/:*?<>\"|][^\\\\/:*?<>\"|]*)+$");
		std::regex nRegex(pattern);
		if (!std::regex_match(path, nRegex))
		{
			return false;
		}*/
		size_t pos = path.find_last_of("/\\");
		if (pos == std::string::npos)
		{
			return false;
		}
		director = path.substr(0, pos + 1);
		return true;
	}

	bool dir::GetFileName(const std::string& fullName, std::string& fileName)
	{
		size_t pos = fullName.find_last_of("/\\");
		if (pos == std::string::npos)
		{
			return false;
		}
		fileName = fullName.substr(pos + 1);
		return true;
	}

	bool dir::GetDirAndFileName(const std::string& path, std::string& director, std::string& fileName)
	{
		/* std::string pattern("^[a-zA-Z]:|([\\\\/]|[^\\s\\\\/:*?<>\"|][^\\\\/:*?<>\"|]*)+$");
		 std::regex nRegex(pattern);
		 if (!std::regex_match(path, nRegex))
		 {
			 return false;
		 }*/
		size_t pos = path.find_last_of("/\\");
		if (pos == std::string::npos)
		{
			return false;
		}
		director = path.substr(0, pos + 1);
		fileName = path.substr(pos + 1);
		return true;
	}
}
