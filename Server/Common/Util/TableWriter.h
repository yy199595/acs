#pragma once
#include <fstream>
#include <string>
namespace Sentry
{
    class TableWriter
    {
    public:
        TableWriter(const std::string &path) {}

    private:
        const std::string mPath;
    };
}// namespace Sentry