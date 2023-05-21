#pragma once

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


#include "../common/util.hpp"
#include "../common/log.hpp"

//只提供编译服务
namespace ns_compiler
{
    //引入路径拼接功能
    using namespace ns_util;
    using namespace ns_log;
    class Compiler
    {
    public:
        Compiler(){};
        ~Compiler(){}

        static bool Compile(const std::string& file_name)
        {
           pid_t pid = fork(); 
           if(pid<0)
           {
                LOG(ERROR)<<"内部错误,创建子进程失败"<<"\n";
                return false;
           }
           else if(pid == 0)
           {
                //子进程
                umask(0);
                int _stderr = open(PathUtil::Stderr(file_name).c_str(), O_CREAT | O_WRONLY, 0644); //110 100 100
                if(_stderr<0)
                {
                    LOG(WARNING)<<"没有成功形成stderr文件"<<"\n";
                    exit(1);
                }
                dup2(_stderr, 2);
                execlp("g++","g++", "-o", PathUtil::Exe(file_name).c_str(),PathUtil::Src(file_name).c_str(),"-std=c++11", NULL);
               // 执行完成后，这里判断有没有形成.exe的可执行程序，如果形成，说明编译成功
               // 如果没有形成，需要将错误信息重定向到文件中
                LOG(ERROR)<<"g++编译失败，可能是参数错误"<<"\n";
                exit(1);    
           }
           else
           {
                waitpid(pid, nullptr,0);
                if(FileUtil::IsFileExists(PathUtil::Exe(file_name).c_str()))
                {
                    LOG(INFO)<<PathUtil::Src(file_name)<<"编译成功"<<"\n";
                    return true;
                }
                LOG(ERROR)<<"编译失败，没有形成可执行程序"<<"\n";
                return false;
           }
        }
    };
}
