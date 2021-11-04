#include "JsonHelper.h"

namespace GameKeeper
{
    RapidJsonWriter::RapidJsonWriter()
        : jsonWriter(strBuf)
    {
        this->jsonWriter.StartObject();
    }


    bool RapidJsonWriter::AddParameter(const char *key, const int value)
    {
        return jsonWriter.Key(key) && jsonWriter.Int(value);
    }

    bool RapidJsonWriter::AddParameter(const char *key, const double value)
    {
        return jsonWriter.Key(key) &&
               jsonWriter.Double(value);

    }

    bool RapidJsonWriter::AddParameter(const char *key, const char *value)
    {
        return this->jsonWriter.Key(key) &&
               this->jsonWriter.String(value);
    }

    bool RapidJsonWriter::AddParameter(const char *key, const long long value)
    {
        return jsonWriter.Key(key) &&
               jsonWriter.Int64(value);
    }

    bool RapidJsonWriter::AddParameter(const char *key, const std::string value)
    {
        return jsonWriter.Key(key) &&
               jsonWriter.String(value.c_str(), value.length());
    }

    bool RapidJsonWriter::AddParameter(const char *key, const unsigned int value)
    {

        return jsonWriter.Key(key) &&
               jsonWriter.Uint(value);
    }

    bool RapidJsonWriter::AddParameter(const char *key, const char *value, size_t size)
    {
        return this->jsonWriter.Key(key) &&
               this->jsonWriter.String(value, size);
    }

    bool RapidJsonWriter::AddParameter(const char *key, const unsigned long long value)
    {

        return jsonWriter.Key(key) &&
               jsonWriter.Uint64(value);
    }

    bool RapidJsonWriter::AddParameter(const char *key, const std::set<std::string> &value)
    {
        if (jsonWriter.Key(key) && jsonWriter.StartArray())
        {
            for (const auto &str: value)
            {
                jsonWriter.String(str.c_str(), str.size());
            }
            return jsonWriter.EndArray();
        }
        return false;
    }

    bool RapidJsonWriter::AddParameter(const char *key, const std::vector<std::string> &value)
    {
        if (jsonWriter.Key(key) && jsonWriter.StartArray())
        {
            for (const auto & str : value)
            {
                jsonWriter.String(str.c_str(), str.size());
            }
            return jsonWriter.EndArray();
        }
        return false;
    }

    bool RapidJsonWriter::AddParameter(const char *key, const google::protobuf::Message &value)
    {
        std::string buffer;
        if (value.SerializePartialToString(&buffer))
        {
            return this->jsonWriter.Key(key) &&
                   this->jsonWriter.String(buffer.c_str(), buffer.size());
        }
        return false;
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
        if(this->jsonWriter.EndObject())
        {
            const char *str = this->strBuf.GetString();
            const size_t lenght = this->strBuf.GetSize();
            os.write(str, lenght);
            return true;
        }

        return false;
    }

    bool RapidJsonWriter::WriterToStream(std::string &os)
    {
        if (this->jsonWriter.EndObject())
        {
            os.clear();
            const char *str = this->strBuf.GetString();
            const size_t lenght = this->strBuf.GetSize();
            os.append(str, lenght);
            return true;
        }
        return false;
    }

    bool RapidJsonWriter::AddParameter(const char *key)
    {
        return jsonWriter.Key(key) &&
               jsonWriter.Null();
    }
}// namespace GameKeeper

namespace GameKeeper
{
    bool RapidJsonReader::TryParse(const char *str, size_t size)
    {
        if (!document.Parse(str, size).HasParseError())
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
        if (!document.Parse(data, size).HasParseError())
        {
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, int &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsInt())
        {
            data = iter->value.GetInt();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, bool &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsBool())
        {
            data = iter->value.GetBool();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, short &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsNumber())
        {
            data = (short) iter->value.GetInt();
            return true;
        }
        return false;
    }

    bool RapidJsonReader::TryGetValue(const char *key, unsigned short &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsNumber())
        {
            data = (unsigned short) iter->value.GetInt();
            return true;
        }
        return false;
    }

    bool RapidJsonReader::TryGetValue(const char *key, float &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsFloat())
        {
            data = iter->value.GetFloat();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, double &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsDouble())
        {
            data = iter->value.GetDouble();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, unsigned int &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsUint())
        {
            data = iter->value.GetUint();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, long long &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsInt64())
        {
            data = iter->value.GetInt64();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, unsigned long long &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsUint64())
        {
            data = iter->value.GetUint64();
            return true;
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, std::vector<std::string> &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsArray())
        {
            MemberIter arrayIter = iter->value.MemberBegin();
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
    bool RapidJsonReader::TryGetValue(const char *key, google::protobuf::Message &data)
    {
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsString())
        {
            const char *str = iter->value.GetString();
            const size_t size = iter->value.GetStringLength();
            return data.ParseFromArray(str, size);
        }
        return false;
    }
    bool RapidJsonReader::TryGetValue(const char *key, std::string &data)
    {
        data = "";
        MemberIter iter = document.FindMember(key);
        if (iter != document.MemberEnd() && iter->value.IsString())
        {
            data.append(iter->value.GetString(), iter->value.GetStringLength());
            return true;
        }
        return false;
    }
}// namespace GameKeeper