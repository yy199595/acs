#pragma once
#include <Define/CommonTypeDef.h>
#include <fstream>
#include <google/protobuf/message.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
using namespace std;
namespace GameKeeper
{
    class RapidJsonWriter
    {
    public:
        RapidJsonWriter();
        ~RapidJsonWriter() = default;
    public:
        bool AddParameter(const char *key);
        bool AddParameter(const char *key, const int value);
        bool AddParameter(const char *key, const double value);
        bool AddParameter(const char *key, const char *value);
        bool AddParameter(const char *key, const long long value);
        bool AddParameter(const char *key, const std::string value);
        bool AddParameter(const char *key, const unsigned int value);
        bool AddParameter(const char *key, const char *value, size_t size);
        bool AddParameter(const char *key, const unsigned long long value);
        bool AddParameter(const char *key, const std::set<std::string> &value);
        bool AddParameter(const char *key, const std::vector<std::string> &value);
        bool AddParameter(const char *key, const google::protobuf::Message &value);

    public:
        bool SaveJsonToFile(const char *path);

        bool WriterToStream(std::string & os);
        bool WriterToStream(std::ostream & os);

        size_t GetJsonSize() { return this->strBuf.GetLength();}
    private:
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> jsonWriter;
    };
}// namespace GameKeeper

namespace GameKeeper
{
    class RapidJsonReader
    {
    public:
        bool TryParse(const std::string &str);
        bool TryParse(const char *str, const size_t size);
        bool ReadFromFile(const char *path);

    public:
        bool TryGetValue(const char *key, int &data);
        bool TryGetValue(const char *key, bool &data);
        bool TryGetValue(const char *key, short &data);
        bool TryGetValue(const char *key, float &data);
        bool TryGetValue(const char *key, double &data);
        bool TryGetValue(const char *key, long long &data);
        bool TryGetValue(const char *key, std::string &data);
        bool TryGetValue(const char *key, unsigned int &data);
        bool TryGetValue(const char *key, unsigned short &data);
        bool TryGetValue(const char *key, unsigned long long &data);
        bool TryGetValue(const char *key, std::vector<std::string> &data);
        bool TryGetValue(const char *key, google::protobuf::Message &data);

        std::string &GetJsonString() { return this->mJsonString; }

    private:
        std::string mJsonString;
        rapidjson::Document document;
        typedef rapidjson::Document::MemberIterator MemberIter;
    };
}// namespace GameKeeper