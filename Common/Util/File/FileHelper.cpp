#include "FileHelper.h"
#include <fstream>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include<regex>
#include"Util/Md5/MD5.h"
#include"DirectoryHelper.h"
#pragma warning(disable : 4996)
namespace Helper
{
    namespace File {
        bool FileIsExist(const std::string &path)
		{
#ifdef _WIN32
            return _access(path.c_str(), 0) == 0;
#else
            return access(path.c_str(), F_OK) == 0;
#endif
        }

        bool GetFileName(const std::string& path, std::string& name)
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

        extern bool GetFileType(const std::string &path, std::string &type)
		{
			std::smatch match;
			std::regex pattern("\\.([a-zA-Z0-9]+)$");
			if (std::regex_search(path, match, pattern))
			{
				type = match[1];
				return true;
			}
			return false;
		}

        bool ReadTxtFile(const std::string &path, std::string &outFile)
        {
            std::ifstream fs;
            fs.open(path, std::ios::in);
            if (!fs.is_open())
            {
                return false;
            }
			char buffer[128] = { 0};
			while(!fs.eof())
			{
				fs.read(buffer, sizeof(buffer));
				const size_t count = fs.gcount();
				if(count > 0)
				{
					outFile.append(buffer, count);
				}
			}
            fs.close();
            return true;
        }

        bool ReadTxtFile(const std::string &path, std::string &outFile, std::string & md5)
        {
            std::ifstream fs;
            fs.open(path, std::ios::in);
            if (!fs.is_open())
            {
                return false;
            }
            std::string line;
            md5 = MD5(fs).toString();
            while (std::getline(fs, line))
            {
                outFile.append(line);
                outFile += "\n";
            }
            fs.close();
            return !outFile.empty();
        }

        bool ReadJsonFile(const std::string &path, rapidjson::Document &document)
        {
            std::string outString;
            if (!File::ReadTxtFile(path, outString))
            {
                return false;
            }
            const char * str = outString.c_str();
            const size_t length = outString.size();
            return !document.Parse(str, length).HasParseError();
        }

        bool ReadJsonFile(const std::string &path, rapidjson::Document &document, std::string &md5)
        {
            std::string outString;
            if (File::ReadTxtFile(path, outString))
            {
                md5 = Helper::Md5::GetMd5(outString);
                document.Parse(outString.c_str(), outString.size());
                return !document.HasParseError();
            }
            return false;
        }

        bool ReadTxtFile(const std::string &path, std::vector<std::string> &outLines)
        {
            std::fstream fs;
            fs.open(path, std::ios::in);
            if (fs.is_open()) {
                std::string tempString;
                while (std::getline(fs, tempString)) {
                    outLines.push_back(tempString);
                    tempString = "";
                }
                return true;
            }
            return false;
        }

        bool WriterFile(const std::string &path, const std::string &fileContent)
        {
            std::string nDirector;
            std::string nFileName;
            if (!Directory::GetDirAndFileName(path, nDirector, nFileName)) {
                return false;
            }
            if (!Directory::DirectorIsExist(nDirector)) {
                Directory::MakeDir(nDirector);
            }
            std::fstream fs(path, std::ios::ate | std::ios::out);
            if (!fs.is_open()) {
                return false;
            }
            fs << fileContent;
            fs.close();
            return true;
        }
    }
}// namespace FileHelper