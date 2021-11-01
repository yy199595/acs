#pragma once
#include <string>
#include <vector>
namespace GameKeeper
{
    class CsvLine
    {
    public:
        void Add(const std::string &value);
        bool GetData(size_t index, std::string &value);
        const size_t GetCount() { return this->mLineDatas.size(); }

    public:
        CsvLine &operator<<(const std::string &value);

    private:
        std::vector<std::string> mLineDatas;
    };
}// namespace GameKeeper

namespace GameKeeper
{
    class CsvFileReader
    {
    public:
        CsvFileReader(const std::string &path);
        ~CsvFileReader();

    public:
        const size_t GetCount() { return this->mAllLines.size(); }
        CsvLine *GetLineData(size_t index);

    private:
        const std::string &mPath;
        std::vector<CsvLine *> mAllLines;
    };
}// namespace GameKeeper

namespace GameKeeper
{
    class CsvFileWriter
    {
    public:
        CsvFileWriter();
        ~CsvFileWriter();

    public:
        void Add(CsvLine *lineData);
        bool Save(const std::string &path);

    private:
        std::vector<CsvLine *> mAllLines;
    };
}// namespace GameKeeper