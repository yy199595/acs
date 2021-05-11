#include"MysqlDefine.h"
namespace SoEasy
{
	bool MysqlQueryLine::AddMember(const std::string & key, const std::string & value)
	{
		if (this->mContent.find(key) == this->mContent.end())
		{
			this->mContent.insert(std::make_pair(key, value));
			return true;
		}
		return false;
	}

	const int MysqlQueryLine::GetInt32Field(const char * key)
	{
		auto iter = this->mContent.find(key);
		return iter != this->mContent.end() ? std::stoi(iter->second) : 0;
	}
	const float MysqlQueryLine::GetFloat32Field(const char * key)
	{
		auto iter = this->mContent.find(key);
		return iter != this->mContent.end() ? std::stof(iter->second) : 0;
	}
	const double MysqlQueryLine::GetFloat64Field(const char * key)
	{
		auto iter = this->mContent.find(key);
		return iter != this->mContent.end() ? std::stod(iter->second) : 0;
	}
	const long long MysqlQueryLine::GetInt64Field(const char * key)
	{
		auto iter = this->mContent.find(key);
		return iter != this->mContent.end() ? std::stoll(iter->second) : 0;
	}
	const std::string & MysqlQueryLine::GetStringField(const char * key)
	{
		static std::string null = "";
		auto iter = this->mContent.find(key);
		return iter != this->mContent.end() ? iter->second : null;
	}


	bool MysqlQueryData::AddFieldName(const char * name, size_t len, int pos)
	{
		const std::string key(name, len);
		auto iter = this->mFiledMap.find(key);
		if (iter == this->mFiledMap.end())
		{
			this->mFiledMap.insert(std::make_pair(key, pos));
			return true;
		}
		return false;
	}

	void MysqlQueryData::AddFieldContent(int row, const char * data, size_t len)
	{		
		if (row >= this->mContentVector.size())
		{
			std::vector<std::string> vec;
			this->mContentVector.push_back(vec);
		}
		this->mContentVector[row].emplace_back(std::string(data, len));
	}

	void MysqlQueryData::DebugPrint()
	{
		std::stringstream ss;
		for (auto iter = this->mFiledMap.begin(); iter != this->mFiledMap.end(); iter++)
		{
			ss << iter->first << "\t";
		}
		std::cout << ss.str() << std::endl;
		
		for (size_t x = 0; x < this->mContentVector.size(); x++)
		{
			ss.clear(); ss.str("");
			for (size_t y = 0; y < this->mContentVector[x].size(); y++)
			{
				ss << this->mContentVector[x][y] << "\t";
			}
			std::cout << ss.str() << std::endl;
		}

	}

	std::shared_ptr<MysqlQueryLine> MysqlQueryData::GetLineData(size_t col)
	{
		if (col < 0 || col > this->mContentVector.size())
		{
			return nullptr;
		}
		std::vector<std::string> & data = this->mContentVector[col];
		std::shared_ptr<MysqlQueryLine> lineData = std::make_shared<MysqlQueryLine>();
		for (auto iter = this->mFiledMap.begin(); iter != this->mFiledMap.end(); iter++)
		{
			const std::string & key = iter->first;
			if (iter->second < data.size())
			{
				lineData->AddMember(key, data[iter->second]);
			}		
		}
		return lineData;
	}
}