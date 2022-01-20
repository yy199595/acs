#pragma once
#include <Define/CommonTypeDef.h>
#include <fstream>
#include <google/protobuf/message.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
using namespace std;
namespace Sentry
{
    class RapidJsonWriter
    {
    public:
        RapidJsonWriter();

        ~RapidJsonWriter() = default;

    public:
        bool Add(const char *key);

        bool Add(const char *key, int value);

        bool Add(const char *key, double value);

        bool Add(const char *key, const char *value);

        bool Add(const char *key, long long value);

        bool Add(const char *key, const std::string &value);

        bool Add(const char *key, unsigned int value);

        bool Add(const char *key, const char *value, size_t size);

        bool Add(const char *key, unsigned long long value);

        bool Add(const char *key, const std::set<std::string> &value);

        bool Add(const char *key, const std::vector<std::string> &value);

        bool Add(const char *key, const google::protobuf::Message &value);

    public:
        bool StartArray(const char *key);

		bool StartObject();
        bool StartObject(const char *key);

    public:
        bool EndArray() { return this->mJsonWriter.EndArray(); }

        bool EndObject() { return this->mJsonWriter.EndObject(); }

    public:
        const char * GetData(size_t & size) const;
    public:
        bool SaveJsonToFile(const char *path);

        virtual bool WriterToStream(std::string &os);

        virtual bool WriterToStream(std::ostream &os);

    protected:
        rapidjson::StringBuffer mStringBuf;
        rapidjson::Writer<rapidjson::StringBuffer> mJsonWriter;
    };
}// namespace Sentry

namespace Sentry
{
    class RapidJsonReader
    {
    public:
        bool TryParse(const std::string &str);
        bool TryParse(const char *str, size_t size);
        bool ReadFromFile(const char *path);
    public:
        bool TryGetValue(const char *key, int &data) const;
        bool TryGetValue(const char *key, bool &data) const;
        bool TryGetValue(const char *key, short &data) const;
        bool TryGetValue(const char *key, float &data) const;
        bool TryGetValue(const char *key, double &data) const;
        bool TryGetValue(const char *key, long long &data) const;
        bool TryGetValue(const char *key, std::string &data) const;
        bool TryGetValue(const char *key, unsigned int &data) const;
        bool TryGetValue(const char *key, unsigned short &data) const;
        bool TryGetValue(const char *key, unsigned long long &data) const;
        bool TryGetValue(const char *key, std::vector<std::string> &data) const;
        bool TryGetValue(const char *key, google::protobuf::Message &data) const;

    private:
        rapidjson::Value * mJsonValue;
		rapidjson::Document mDdocument;
    };
}// namespace Sentry