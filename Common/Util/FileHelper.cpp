#include "FileHelper.h"
#include <fstream>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
#include"MD5.h"
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

        bool ReadTxtFile(const std::string &path, std::string &outFile)
        {
            std::fstream fs;
            fs.open(path, std::ios::in);
            if (fs.is_open()) {
                std::string line;
                while (std::getline(fs, line)) {
                    outFile.append(line);
                }
                fs.close();
                return true;
            }
            return false;
        }

        bool ReadJsonFile(const std::string &path, rapidjson::Document &document)
        {
            std::string outString;
            if (File::ReadTxtFile(path, outString)) {
                document.Parse(outString.c_str(), outString.size());
                return !document.HasParseError();
            }
            return false;
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