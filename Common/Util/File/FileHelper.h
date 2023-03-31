#pragma once

#include <string>
#include <vector>
#include "rapidjson/document.h"
namespace Helper
{
    namespace File
    {
        extern bool FileIsExist(const std::string &path);

		extern bool GetFileType(const std::string & path, std::string & type);

        extern bool ReadTxtFile(const std::string &path, std::string &outFile);

        extern bool ReadTxtFile(const std::string &path, std::string &outFile, std::string & md5);

        extern bool ReadJsonFile(const std::string &path, rapidjson::Document &document);

        extern bool ReadJsonFile(const std::string &path, rapidjson::Document &document, std::string &md5);

        extern bool ReadTxtFile(const std::string &path, std::vector<std::string> &outLines);

        extern bool WriterFile(const std::string &path, const std::string &fileContent);
    }

}// namespace FileHelper
