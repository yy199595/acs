#pragma once
#include<string>
#include<vector>
#include<rapidjson/document.h>

namespace FileHelper
{
	extern bool FileIsExist(const std::string path);

	extern bool ReadTxtFile(const std::string path, std::string & outFile);

	extern bool ReadJsonFile(const std::string path, rapidjson::Document & document);

	extern bool ReadTxtFile(const std::string path, std::vector<std::string> & outLines);

	extern bool WriterFile(const std::string path, const std::string & fileContent);

}

