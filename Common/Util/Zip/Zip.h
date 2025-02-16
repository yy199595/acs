//
// Created by yy on 2025/2/5.
//

#pragma once
#include <string>
#include <vector>
namespace help
{
    namespace zip
    {
		extern bool Unzip(const std::string & dir, const std::string & path);
        extern bool Create(const std::string & dir, const std::string & path);
		extern bool Create(const std::string & dir, const std::vector<std::string> & files, const std::string & path);
    }
}