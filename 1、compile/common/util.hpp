#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

namespace ns_util
{
    class TimeUtil
    {
    public:
        static std::string GetTimeStamp()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            return std::to_string(_time.tv_sec);
        }
    };

    const std::string temp_path = "./temp/";
    
    class PathUtil
    {
    public:
        static std::string AddSuffix(const std::string& file_name, const std::string& suffix)
        {
            std::string path_name = temp_path+file_name;
            path_name+=suffix;
            return path_name;
        }
        // 构建源文件路径+后缀的完整文件名
        // ./temp/1234.cpp
        static std::string Src(const std::string& file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }
        
        //构建可执行程序的完整路径+后缀名
        //1234->./temp/1234.exe
        static std::string Exe(const std::string& file_name)
        {
            return AddSuffix(file_name, ".exe");
        }

        //构建该程序对应的标准错误的完整路径+后缀名
        //1234->./temp/1234.stderr
        static std::string Stderr(const std::string& file_name)
        {
            return AddSuffix(file_name, ".stderr");
        }
    };

    class FileUtil
    {
    public:
        static bool IsFileExists(const std::string& file_name)
        {
            struct stat buf;
            if(stat(file_name.c_str(), &buf)==0)
            {
                return true;
            }
            return false;
        }
    };


}
