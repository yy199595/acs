#include"CsvHelper.h"
#include"FileHelper.h"
#include"StringHelper.h"
#include<fstream>
namespace SoEasy
{
	void CsvLine::Add(const std::string & value)
	{
		this->mLineDatas.push_back(value);
	}
	bool CsvLine::GetData(size_t index, std::string & value)
	{
		if (this->mLineDatas.size() > 0 && index >= 0 && index < this->mLineDatas.size())
		{
			value = this->mLineDatas[index];
			return true;
		}
		return false;
	}
	CsvLine & CsvLine::operator<<(const std::string & value)
	{
		this->mLineDatas.push_back(value);
		return (*this);
	}
	
}
namespace SoEasy
{
	CsvFileReader::CsvFileReader(const std::string & path)
		: mPath(path)
	{
		std::vector<std::string> tempArray;
		std::vector<std::string> fileContent;
		if (FileHelper::ReadTxtFile(path, fileContent))
		{
			for (size_t index = 0; index < fileContent.size(); index++)
			{
				const std::string & data = fileContent[index];
				StringHelper::SplitString(data, "\t", tempArray);
				if (tempArray.empty())
				{
					continue;
				}
				CsvLine * lineData = new CsvLine();
				for (const std::string & value : tempArray)
				{
					lineData->Add(value);
				}
			}
		}
	}
	CsvFileReader::~CsvFileReader()
	{
		for (size_t index = 0; index < this->mAllLines.size(); index++)
		{
			delete this->mAllLines[index];
		}
		this->mAllLines.clear();
	}
	CsvLine * CsvFileReader::GetLineData(size_t index)
	{
		if (this->mAllLines.size() > 0 && index >= 0 && index < this->mAllLines.size())
		{
			return this->mAllLines[index];
		}
		return nullptr;
	}
}

namespace SoEasy
{
	CsvFileWriter::CsvFileWriter()
	{

	}

	CsvFileWriter::~CsvFileWriter()
	{
		for (size_t index = 0; index < this->mAllLines.size(); index++)
		{
			delete this->mAllLines[index];
		}
		this->mAllLines.clear();
	}

	bool CsvFileWriter::Save(const std::string & path)
	{
		std::fstream fs(path, std::ios::out);
		if (!fs.is_open())
		{
			return false;
		}
		for (size_t index = 0; index < this->mAllLines.size(); index++)
		{
			CsvLine * lineData = this->mAllLines[index];
			for (size_t x = 0; x < lineData->GetCount(); x++)
			{
				std::string value;
				if (lineData->GetData(x, value))
				{
					fs << value << "\t";
				}
			}
			fs << "\n";
		}
		fs.close();
		return true;
	}
}
