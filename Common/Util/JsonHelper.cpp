#include "JsonHelper.h"

namespace GameKeeper
{
    RapidJsonWriter::RapidJsonWriter()
        : mJsonWriter(mStringBuf)
    {
        this->mJsonWriter.StartObject();
    }


    bool RapidJsonWriter::Add(const char *key, int value)
    {
        return mJsonWriter.Key(key) && mJsonWriter.Int(value);
    }

    bool RapidJsonWriter::Add(const char *key, double value)
    {
        return mJsonWriter.Key(key) &&
               mJsonWriter.Double(value);

    }

    bool RapidJsonWriter::Add(const char *key, const char *value)
    {
        return this->mJsonWriter.Key(key) &&
               this->mJsonWriter.String(value);
    }

    bool RapidJsonWriter::Add(const char *key, long long value)
    {
        return mJsonWriter.Key(key) &&
               mJsonWriter.Int64(value);
    }

    bool RapidJsonWriter::Add(const char *key, const std::string & value)
    {
        return mJsonWriter.Key(key) &&
               mJsonWriter.String(value.c_str(), value.length());
    }

    bool RapidJsonWriter::Add(const char *key, unsigned int value)
    {

        return mJsonWriter.Key(key) &&
               mJsonWriter.Uint(value);
    }

    bool RapidJsonWriter::Add(const char *key, const char *value, size_t size)
    {
        return this->mJsonWriter.Key(key) &&
               this->mJsonWriter.String(value, size);
    }

    bool RapidJsonWriter::Add(const char *key, unsigned long long value)
    {

        return mJsonWriter.Key(key) &&
               mJsonWriter.Uint64(value);
    }

    bool RapidJsonWriter::Add(const char *key, const std::set<std::string> &value)
    {
        if (mJsonWriter.Key(key) && mJsonWriter.StartArray())
        {
            for (const auto &str: value)
            {
                mJsonWriter.String(str.c_str(), str.size());
            }
            return mJsonWriter.EndArray();
        }
        return false;
    }

    bool RapidJsonWriter::Add(const char *key, const std::vector<std::string> &value)
    {
        if (mJsonWriter.Key(key) && mJsonWriter.StartArray())
        {
            for (const auto & str : value)
            {
                mJsonWriter.String(str.c_str(), str.size());
            }
            return mJsonWriter.EndArray();
        }
        return false;
    }

    bool RapidJsonWriter::Add(const char *key, const google::protobuf::Message &value)
    {
        std::string buffer;
        if (value.SerializePartialToString(&buffer))
        {
            return this->mJsonWriter.Key(key) &&
                   this->mJsonWriter.String(buffer.c_str(), buffer.size());
        }
        return false;
    }

    bool RapidJsonWriter::StartArray(const char *key)
    {
        return this->mJsonWriter.Key(key) &&
               this->mJsonWriter.StartArray();
    }

	bool RapidJsonWriter::StartObject()
	{
		return this->mJsonWriter.StartObject();
	}

    bool RapidJsonWriter::StartObject(const char *key)
    {
        return this->mJsonWriter.Key(key) &&
               this->mJsonWriter.StartObject();
    }

    bool RapidJsonWriter::SaveJsonToFile(const char *path)
    {
        std::ofstream savefile(path, ios::out | ios::binary);
        if (savefile.is_open())
        {
            this->WriterToStream(savefile);
            savefile.close();
            return true;
        }
        return false;
    }

    bool RapidJsonWriter::WriterToStream(std::ostream & os)
    {
        if(this->mJsonWriter.EndObject())
        {
            const char *str = this->mStringBuf.GetString();
            const size_t size = this->mStringBuf.GetSize();
            os.write(str, size);
            return true;
        }

        return false;
    }

    const char *RapidJsonWriter::GetData(size_t & size) const
    {
        size = this->mStringBuf.GetSize();
        return this->mStringBuf.GetString();
    }

    bool RapidJsonWriter::WriterToStream(std::string &os)
    {
        if (this->mJsonWriter.EndObject())
        {
            os.clear();
            const char *str = this->mStringBuf.GetString();
            const size_t lenght = this->mStringBuf.GetSize();
            os.append(str, lenght);
            return true;
        }
        return false;
    }

    bool RapidJsonWriter::Add(const char *key)
    {
        return mJsonWriter.Key(key) &&
               mJsonWriter.Null();
    }
}// namespace GameKeeper

namespace GameKeeper
{
    bool RapidJsonReader::TryParse(const char *str, size_t size)
    {
        if (!mDdocument.Parse(str, size).HasParseError())
        {
            return true;
        }
        return false;
    }
    bool RapidJsonReader::ReadFromFile(const char *path)
    {
        std::ifstream readfs(path, ios::in | ios::binary);
        if (readfs)
        {
            std::string data;
            readfs >> data;
            return this->TryParse(data);
        }
        return false;
    }
    bool RapidJsonReader::TryParse(const std::string &str)
    {
        const char * data = str.c_str();
        const size_t size = str.length();
        if (!mDdocument.Parse(data, size).HasParseError())
        {
            return true;
        }
        std::cout << "json : " << str << " parse error" << std::endl;
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, int &data) const
    {
        auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsInt())
        {
            data = iter->value.GetInt();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, bool &data) const
    {
        auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsBool())
        {
            data = iter->value.GetBool();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, short &data) const
    {
		auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsNumber())
        {
            data = (short) iter->value.GetInt();
            return true;
        }
        return false;
    }

    bool RapidJsonReader::TryGetValue(const char *key, unsigned short &data) const
    {
		auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsNumber())
        {
            data = (unsigned short) iter->value.GetInt();
            return true;
        }
        return false;
    }

    bool RapidJsonReader::TryGetValue(const char *key, float &data) const
    {
        auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsFloat())
        {
            data = iter->value.GetFloat();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, double &data) const
    {
		auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsDouble())
        {
            data = iter->value.GetDouble();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, unsigned int &data) const
    {
        auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsUint())
        {
            data = iter->value.GetUint();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, long long &data) const
    {
        auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsInt64())
        {
            data = iter->value.GetInt64();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, unsigned long long &data) const
    {
        auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsUint64())
        {
            data = iter->value.GetUint64();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, std::vector<std::string> &data) const
    {
        auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsArray())
        {
            auto arrayIter = iter->value.MemberBegin();
            for (; arrayIter != iter->value.MemberEnd(); arrayIter++)
            {
                if (arrayIter->value.IsString())
                {
                    const char *str = arrayIter->value.GetString();
                    const size_t size = arrayIter->value.GetStringLength();
                    data.emplace_back(str, size);
                }
            }
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, google::protobuf::Message &data) const
    {
        auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsString())
        {
            const char *str = iter->value.GetString();
            const size_t size = iter->value.GetStringLength();
            return data.ParseFromArray(str, size);
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, std::string &data) const
    {	
        auto iter = mDdocument.FindMember(key);
        if (iter != mDdocument.MemberEnd() && iter->value.IsString())
        {
			data.clear();
            data.append(iter->value.GetString(), iter->value.GetStringLength());
            return true;
        }
        return false;
    }
}// namespace GameKeeper