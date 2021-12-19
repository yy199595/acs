#include "DirectoryHelper.h"
#include <regex>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#define MakeDirectory(path) _mkdir(path.c_str())
#else
#define MakeDirectory(path) mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH)
#endif
#define PATH_MAX_LENGHT 1024
namespace Helper
{
    namespace Directory
    {
        bool MakeDir(const std::string dir)
        {
            if (Directory::DirectorIsExist(dir)) {
                return true;
            }

            for (size_t index = 0; index < dir.size(); index++) {
                if (dir[index] == '/' || dir[index] == '\\') {
                    const std::string path = dir.substr(0, index);
                    MakeDirectory(path);
                }
            }
            return MakeDirectory(dir) != -1;
        }

    bool DeleteDir(const std::string dir)
    {
#ifdef _WIN32
        return _rmdir(dir.c_str()) != -1;
#else
        return rmdir(dir.c_str()) != -1;
#endif
    }

    bool DirectorIsExist(const std::string dir)
    {
#ifdef _WIN32
        return _access(dir.c_str(), 0) == 0;
#else
        return access(dir.c_str(), F_OK) == 0;
#endif
    }

    bool GetFilePaths(const std::string path, std::vector<std::string> &paths)
    {
#ifdef _WIN32
        char newpath[PATH_MAX_LENGHT] = {0};
        sprintf_s(newpath, "%s/*.*", path.c_str());
        WIN32_FIND_DATA findFileData;
        HANDLE fileHandle = FindFirstFile(newpath, &findFileData);
        if (fileHandle == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        while (true)
        {
            if (findFileData.cFileName[0] != '.')
            {
                if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    memset(newpath, 0, PATH_MAX_LENGHT);
#ifdef _MSC_VER
                    size_t size = sprintf_s(newpath, "%s/%s", path.c_str(), findFileData.cFileName);
#else
                    size_t size = sprintf(newpath, "%s/%s", path.c_str(), findFileData.cFileName);
#endif
                    GetFilePaths(std::string(newpath, size), paths);
                } else
                {
#ifdef _MSC_VER
                    size_t size = sprintf_s(newpath, "%s/%s", path.c_str(), findFileData.cFileName);
#else
                    size_t size = sprintf(newpath, "%s/%s", path.c_str(), findFileData.cFileName);
#endif

                    paths.push_back(std::string(newpath, size));
                }
            }
            //如果是当前路径或者父目录的快捷方式，或者是普通目录，则寻找下一个目录或者文件
            if (!FindNextFile(fileHandle, &findFileData))
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
            } else if (ptr->d_type == DT_DIR)
            {
                std::string strFile = path + "/" + ptr->d_name;
                GetFilePaths(strFile, paths);
            } else if (ptr->d_type == 8)///file
            {
                std::string strFile = path + "/" + ptr->d_name;
                paths.push_back(strFile);
            }
        }
        return closedir(dir) != -1;
#endif// !_WIN32
    }
    bool GetFilePaths(const std::string path, std::string format, std::vector<std::string> &paths)
    {
        std::vector<std::string> allPaths;
        if (GetFilePaths(path, allPaths))
        {
            if (format[0] == '*') { format = format.substr(1); }
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

	bool GetDirByPath(const std::string path, std::string & director)
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

	bool GetDirAndFileName(const std::string path, std::string &director, std::string &fileName)
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
        director = path.substr(0, pos);
        fileName = path.substr(pos + 1);
        return true;
    }
    }
}
