#include "AssestsFileInfo.h"
#include<CommonUtil/MD5.h>
#include<fstream>
#include<sstream>
namespace SoEasy
{
	AssestsFileInfo::AssestsFileInfo()
	{
		mIsOpen = false;
		this->mFileSize = 0;
	}

	bool AssestsFileInfo::LaodFile(const std::string & dir, const std::string path)
	{
		this->mFileSize = 0;
		this->mFileMd5.clear();
		this->mFileContent.clear();
		std::stringstream mFileContentStream;
		std::fstream nFileStream(path, std::ios::in);
		if (!nFileStream.is_open())
		{
			return false;
		}
		std::string nLineFile;
		while (std::getline(nFileStream, nLineFile))
		{
			if (!nLineFile.empty())
			{
				mFileContentStream << nLineFile << "\n";
			}
			nLineFile.clear();
		}
		nFileStream >> this->mFileContent;
		nFileStream.close();
		this->mFileContent = mFileContentStream.str();

		this->mFilePath = path;
		MD5 md5(this->mFileContent);
		this->mFileMd5 = md5.toString();
		this->mFileSize = this->mFileContent.size();
		this->mFullName = path.substr(dir.size() + 1);
		return true;
	}

	bool AssestsFileInfo::ReadFileContent(std::string & fileContent, size_t size)
	{
		size = size == 0 ? this->mFileSize : size;
		fileContent = this->mFileContent.substr(this->mIndex, this->mIndex + size);
		this->mIndex += size;
		return this->mIndex < this->mFileSize;
	}
}
