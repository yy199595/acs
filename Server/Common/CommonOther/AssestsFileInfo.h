#pragma once
#include<vector>
#include<string>
namespace SoEasy
{
	class AssestsFileInfo
	{
	public:
		AssestsFileInfo();
	public:
		bool LaodFile(const std::string & dir, const std::string path);	
	public:
		size_t GetFileSize() { return this->mFileSize; }
		const std::string & GetMd5() { return this->mFileMd5; }
		void SetFileIndex(size_t index = 0) { mIndex = index; }
		const std::string & GetFullName() { return this->mFullName; }
	public:
		bool ReadFileContent(std::string & fileContent, size_t size);
		const std::string & GetFileContent() { return this->mFileContent; }
	private:
		bool mIsOpen;
		size_t mIndex;
		size_t mFileSize;
		std::string mServer;
		std::string mFormat;
		std::string mFileMd5;
		std::string mFilePath;
		std::string mFullName;
		std::string mAssestDir;
		std::string mFileContent;
	};
}