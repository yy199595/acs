#include "NetworkHelper.h"
#include <regex>
namespace Helper
{
    bool Net::IsIp(const std::string &ip)
    {
        std::regex regExpress(
                R"((?=(\b|\D))(((\d{1,2})|(1\d{1,2})|(2[0-4]\d)|(25[0-5]))\.){3}((\d{1,2})|(1\d{1,2})|(2[0-4]\d)|(25[0-5]))(?=(\b|\D)))");
        return std::regex_match(ip, regExpress);
    }
    bool Net::ParseHttpUrl(const std::string &url, std::string &host, std::string &port, std::string &path)
    {
        std::cmatch what;
        std::string protocol;
        std::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/.*)?");
        if (std::regex_match(url.c_str(), what, pattern)) {
            host = std::string(what[2].first, what[2].second);
            path = std::string(what[4].first, what[4].second);
            protocol = std::string(what[1].first, what[1].second);
            port = std::string(what[3].first, what[3].second);

            if (0 == port.length()) {
                port = "http" == protocol ? "80" : "443";
            }
            return true;
        }
        return false;
    }
}
