#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>


#include "../common/util.hpp"
#include "../common/log.hpp"

namespace ns_runner
{
    using namespace ns_util;
    using namespace ns_log;
    class Runner
    {
    public:
        Runner(){}
        ~Runner(){}
    public:
        // 在运行之间需要对资源做一些限制
        // 包括 时间、空间
        static void setProcLimit(int _cpu_limit, int _mem_limit)
        {
            // 设置CPU的时长
            struct rlimit cpu_rlimit;
            cpu_rlimit.rlim_cur = _cpu_limit;
            cpu_rlimit.rlim_max = RLIM_INFINITY;

            setrlimit(RLIMIT_CPU, &cpu_rlimit);
            
            // 设置限制内存
            struct rlimit mem_rlimit;
            //转成kb
            mem_rlimit.rlim_cur = _mem_limit*1024;
            mem_rlimit.rlim_max = RLIM_INFINITY;
            setrlimit(RLIMIT_AS, &mem_rlimit);
        }

        
        static int Run(const std::string& file_name, int cpu_rlimit, int mem_rlimit)
        {
            std::string _execute = PathUtil::Exe(file_name);
            std::string _stdin  = PathUtil::Stdin(file_name);
            std::string _stdout = PathUtil::Stdout(file_name);
            std::string _stderr = PathUtil::Stderr(file_name);
            umask(0);
            int _stdin_fd = open(_stdin.c_str(), O_CREAT|O_RDONLY, 0644);
            int _stdout_fd = open(_stdout.c_str(), O_CREAT|O_WRONLY, 0644);
            int _stderr_fd = open(_stderr.c_str(), O_CREAT|O_WRONLY, 0644);

            if(_stdin_fd <0 || _stdout_fd<0 || _stderr_fd<0)
            {
                LOG(ERROR)<<"运行时打开文件失败"<<"\n";
                return -1; //-1 代表打开文件失败
            }

            pid_t pid = fork();
            
            if(pid<0)
            {
               LOG(ERROR)<<"运行时创建子进程失败"<<"\n";
               close(_stdin_fd);
               close(_stdout_fd);
               close(_stderr_fd);

               return -2; //-2 代表创建子进程失败
            }
            else if(pid==0)
            {
                // 子进程，先进行重定向，再程序替换
                dup2(_stdin_fd, 0);
                dup2(_stdout_fd,1);
                dup2(_stderr_fd, 2);
                
                setProcLimit(cpu_rlimit, mem_rlimit);
                execl(_execute.c_str(), _execute.c_str(), nullptr);
                exit(1);
            }
            else
            {
                // 父进程
                // 先关闭文件描述符
                
                close(_stdin_fd);
                close(_stdout_fd);
                close(_stderr_fd);
                
                int status=0;
                waitpid(pid, &status, 0);
                LOG(INFO)<<"运行完毕 Info: "<<(status & 0x7F)<<"\n";
                return status & 0x7F;
            }
        }
    };
}
