#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <atomic>
#include <fstream>
#include <boost/algorithm/string.hpp>

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

        // 6-18新增毫秒级
        static std::string GetTimeMs()
        {
            struct timeval _time;
            gettimeofday(&_time, nullptr);
            // 秒*1000 微秒、1000
            return std::to_string(_time.tv_sec * 1000 + _time.tv_usec / 1000);
        }
    };

    const std::string temp_path = "./temp/";

    class PathUtil
    {
    public:
        static std::string AddSuffix(const std::string &file_name, const std::string &suffix)
        {
            std::string path_name = temp_path + file_name;
            path_name += suffix;
            return path_name;
        }
        // 构建源文件路径+后缀的完整文件名
        // ./temp/1234.cpp
        static std::string Src(const std::string &file_name)
        {
            return AddSuffix(file_name, ".cpp");
        }

        // 构建可执行程序的完整路径+后缀名
        // 1234->./temp/1234.exe
        static std::string Exe(const std::string &file_name)
        {
            return AddSuffix(file_name, ".exe");
        }

        // 构建该程序对应的标准错误的完整路径+后缀名
        // 1234->./temp/1234.stderr
        static std::string Stderr(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stderr");
        }

        static std::string CompilerError(const std::string &file_name)
        {
            return AddSuffix(file_name, ".compile_error");
        }

        static std::string Stdin(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdin");
        }

        static std::string Stdout(const std::string &file_name)
        {
            return AddSuffix(file_name, ".stdout");
        }
    };

    class FileUtil
    {
    public:
        static bool IsFileExists(const std::string &file_name)
        {
            struct stat buf;
            if (stat(file_name.c_str(), &buf) == 0)
            {
                return true;
            }
            return false;
        }

        // 形成唯一的文件名
        // 通过毫秒级时间戳+原子性递增唯一值来保证唯一性
        static std::string UniqFileName()
        {
            static std::atomic_uint id(0);
            id++;

            std::string ms = TimeUtil::GetTimeMs();
            std::string uniq_id = std::to_string(id);
            return ms + "_" + uniq_id;
        }

        // 6-18
        // 将content写入到文件target中
        static bool WriteFile(const std::string &target, const std::string &content)
        {
            std::ofstream out(target);
            if (!out.is_open())
            {
                return false;
            }

            out.write(content.c_str(), content.size());
            out.close();
            return true;
        }

        // 6-18 读 将文件中的代码读到string中
        static bool ReadFile(const std::string &target, std::string *content, bool keep = false)
        {
            (*content).clear();

            std::ifstream in(target);

            if (!in.is_open())
            {
                return false;
            }

            std::string line;
            // 一行一行的读
            // getline 不保存行分隔符，有些时候需要保存\n
            // getline 内部重载了强制类型转换
            while (std::getline(in, line))
            {
                (*content) += line;
                (*content) += (keep ? "\n" : ""); // keep为真 加\n keep为false不加\n
            }
            in.close();
            return true;
        }
    };

    class StringUtil
    {
    public:
        /*************************************
         * str: 输入型，目标要切分的字符串
         * target: 输出型，保存切分完毕的结果
         * sep: 指定的分割符
         * **********************************/
        static void SplitString(const std::string &str, std::vector<std::string> *target, const std::string &sep)
        {
            boost::split((*target), str, boost::is_any_of(sep), boost::algorithm::token_compress_on);
        }
    };
}
