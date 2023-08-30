#pragma once
#include <iostream>
#include <string>
#include "util.hpp"

namespace ns_log
{
    using namespace ns_util;

    enum{
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        FATAL
    };

    inline std::ostream& Log(const std::string& level, const std::string& file_name, int line)
    {
        //日志等级
        std::string message = "[";
        message+=level;
        message+="]";
        //报错文件名
        message+="[";
        message+=file_name;
        message+="]";

        //报错行数
        message+="[";
        message+=std::to_string(line);
        message+="]";

        //报错时间戳
        message+="[";
        message+=TimeUtil::GetTimeStamp();
        message+="]";

        std::cout<<message;
        return std::cout;
    }

    #define LOG(level) Log(#level, __FILE__, __LINE__)
}

