#include <sys/stat.h>
#include "FileHelper.h"
#include <fstream>
#ifdef __OS_WIN__
#include <io.h>
#include <codecvt>
#define stat _stat
#else
#include <unistd.h>
#endif

#include<regex>
#include"fmt.h"
#include"DirectoryHelper.h"

#pragma warning(disable : 4996)


namespace help
{
	bool fs::GetFileLine(const std::string& path, size_t& size)
	{
		std::ifstream fs;
		fs.open(path, std::ios::in | std::ios::binary);
		if (!fs.is_open())
		{
			return false;
		}
		std::string data;
		while (std::getline(fs, data))
		{
			size++;
			data.clear();
		}
		return true;
	}

	bool fs::GetFileSize(const std::string& path, size_t& size)
	{
		std::ifstream fs;
		fs.open(path, std::ios::in | std::ios::binary);
		if (!fs.is_open())
		{
			return false;
		}
		fs.seekg(0, std::ios::end);
		size = fs.tellg();
		fs.close();
		return true;
	}

	bool fs::GetFileSize(const std::string& path, std::string& str)
	{
		size_t size = 0;
		if(!help::fs::GetFileSize(path, size))
		{
			return false;
		}

		double fileSize = (double)size;
		if(fileSize >= help::fs::GB)
		{
			double val = fileSize / help::fs::GB;
			str = fmt::format("{:.1f}GB", (float)val);
		}
		else if(fileSize >= help::fs::MB)
		{
			double val = fileSize / help::fs::MB;
			str = fmt::format("{:.1f}MB", (float)val);
		}
		else if(fileSize >= help::fs::KB)
		{
			double val = fileSize / help::fs::KB;
			str = fmt::format("{:.1f}KB", (float)val);
		}
		else
		{
			str = fmt::format("{}B", fileSize);
		}
		return true;
	}

	bool fs::GetFileSize(std::fstream& fs, size_t& size)
	{
		if (!fs.is_open())
		{
			return false;
		}
		fs.seekg(0, std::ios::end);
		size = fs.tellg();
		fs.seekg(0, std::ios::beg);
		return true;
	}

	bool fs::FileIsExist(const std::string& path)
	{
#ifdef _WIN32
		if(_access(path.c_str(), 0) == 0)
		{
			return true;
		}
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::wstring newPath = converter.from_bytes(path);
		return  _waccess(newPath.c_str(), 0) == 0;
#else
		return access(path.c_str(), F_OK) == 0;
#endif
	}

	long long fs::GetLastWriteTime(const std::string& path)
	{
		struct stat result{};
		if (stat(path.c_str(), &result) == 0)
		{
			return result.st_mtime;
		}
		return 0;
	}

	bool fs::GetFileName(const std::string& path, std::string& name)
	{
		std::regex pattern("/([^/]+)\\.[\\w]+$");
		std::smatch match;
		if (!std::regex_search(path, match, pattern))
		{
			return false;
		}
		name = match[1];
		return true;
	}

	extern bool fs::GetFileType(const std::string& path, std::string& type)
	{
		std::smatch match;
		std::regex pattern("\\.([a-zA-Z0-9]+)$");
		if (std::regex_search(path, match, pattern))
		{
			type = match[1];
			std::transform(type.begin(), type.end(), type.begin(), ::tolower);
			return true;
		}
		return false;
	}

	bool fs::ReadTxtFile(const std::string& path, std::string& outFile)
	{
		std::ifstream fs;
		fs.open(path, std::ios::in);
		if(!fs.is_open())
		{
#ifdef __OS_WIN__
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			std::wstring newPath = converter.from_bytes(path);
			fs.open(newPath, std::ios::in);
			if (!fs.is_open())
			{
				return false;
			}
#else
			return false;
#endif
		}
		char buffer[128] = { 0 };
		do
		{
			fs.read(buffer, sizeof(buffer));
			size_t count = fs.gcount();
			if (count > 0)
			{
				outFile.append(buffer, count);
			}
		}
		while (!fs.eof());
		fs.close();
		return true;
	}

	bool fs::ReadTxtFile(const std::string& path, std::vector<std::string>& outLines)
	{
		std::fstream fs;
		fs.open(path, std::ios::in);
		if (fs.is_open())
		{
			std::string tempString;
			while (std::getline(fs, tempString))
			{
				outLines.push_back(tempString);
				tempString = "";
			}
			return true;
		}
		return false;
	}

	bool fs::WriterFile(const std::string& path, const std::string& fileContent)
	{
		std::string nDirector;
		std::string nFileName;
		if (!dir::GetDirAndFileName(path, nDirector, nFileName))
		{
			return false;
		}
		if (!dir::DirectorIsExist(nDirector))
		{
			dir::MakeDir(nDirector);
		}
		std::fstream fs(path, std::ios::ate | std::ios::out | std::ios::binary);
		if (!fs.is_open())
		{
			return false;
		}
		fs.write(fileContent.c_str(), (std::streamsize)fileContent.size());
		fs.close();
		return true;
	}

	bool fs::ChangeName(const std::string& path, const std::string& name)
	{
		const char * oldFilePath = path.c_str();
		const char* directoryPath = strrchr(oldFilePath, '/');
		if (directoryPath)
		{
			++directoryPath;
			std::string director;
			char newFilePath[256] = { 0 };
			snprintf(newFilePath, sizeof(newFilePath), "%.*s%s", static_cast<int>(directoryPath - oldFilePath),
					oldFilePath, name.c_str());
			if(help::dir::GetDirByPath(newFilePath, director))
			{
				help::dir::MakeDir(director);
			}
			std::remove(newFilePath);
			return std::rename(oldFilePath, newFilePath) == 0;
		}
		return false;
	}
}// namespace FileHelper