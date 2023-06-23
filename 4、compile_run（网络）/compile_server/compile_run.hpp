#pragma once
#include "compiler.hpp"
#include "runner.hpp"
#include "../common/log.hpp"
#include "../common/util.hpp"

#include <signal.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>

namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;


    class CompileAndRun
    {
    public:
        static void RemoveTempFile(const std::string &file_name)
        {
            //清理源文件
            std::string _src = PathUtil::Src(file_name);
            if(FileUtil::IsFileExists(_src))
                unlink(_src.c_str());
            //清理编译错误文件
            std::string _compiler_error=PathUtil::CompilerError(file_name);
            if(FileUtil::IsFileExists(_compiler_error))
                unlink(_compiler_error.c_str());
            //清理可执行程序
            std::string _execute=PathUtil::Exe(file_name);
            if(FileUtil::IsFileExists(_execute))
                unlink(_execute.c_str());
            //清理输入
            std::string _stdin=PathUtil::Stdin(file_name);
            if(FileUtil::IsFileExists(_stdin))
                unlink(_stdin.c_str());
            //清理输出
            std::string _stdout=PathUtil::Stdout(file_name);
            if(FileUtil::IsFileExists(_stdout))
                unlink(_stdout.c_str());
            //清理错误
            std::string _stderr=PathUtil::Stderr(file_name);
            if(FileUtil::IsFileExists(_stderr))
                unlink(_stderr.c_str());
        }

        static std::string CodeToDesc(int code, const std::string& file_name)
        {
            std::string desc;
            switch(code)
            {
            case 0:
                desc="编译运行成功";
                break;
            case -1:
                desc="提交的代码为空";
                break;
            case -2:
                desc="未知错误";
                break;
            case -3:
                //代码编译时出错
                FileUtil::ReadFile(PathUtil::CompilerError(file_name), &desc, true);
                break;
            case SIGABRT: //6
                desc="内存超过范围";
                break;
            case SIGXCPU: //24
                desc="CPU使用超时";
                break;
            case SIGFPE: //8
                desc="浮点数溢出";
                break;
            default:
                desc="未知"+std::to_string(code);
                break;
            }
            return desc;
        }
        static void Start(const std::string &in_json, std::string *out_json)
        {
           // 解析 in_json得到 代码 输入 限制条件
           Json::Value in_value;
           Json::Reader reader;
           reader.parse(in_json, in_value);

           std::string code = in_value["code"].asString();
           std::string input = in_value["input"].asString();
           int cpu_limit = in_value["cpu_limit"].asInt();
           int mem_limit = in_value["mem_limit"].asInt();

            // 根据code 生成一份文件，该文件名必须要有唯一性
            
           int status_code=0;
           Json::Value out_value; 
           int run_result = 0;

           std::string file_name; // code文件名
        
           if(code.size()==0)
           {
               status_code = -1; //代表代码为空
               goto END;
           }

            
           // 形成的文件名要具有唯一性，只是文件名，没有前缀路径和后缀
           file_name = FileUtil::UniqFileName();

           // 文件名有了，接下来是根据文件名，形成src源文件，并将code的内容写入到源文件中
           if(!FileUtil::WriteFile(PathUtil::Src(file_name), code))
           {
               status_code=-2; //未知错误
               goto END;
           }

           
           //形成源文件之后开始编译
           if(!Compiler::Compile(file_name))
           {
               status_code=-3; //代码编译发生错误
               goto END;
           }

           //编译完成之后运行
           run_result=Runner::Run(file_name, cpu_limit, mem_limit);
           if(run_result<0)
           {
               status_code=-2;//未知错误
           }
           else if(run_result>0)
           {
               //程序运行崩溃
               status_code=run_result;
           }
           else
           {
               status_code=0; //程序运行成功
           }
END:
           out_value["status"]=status_code;
           out_value["reason"]=CodeToDesc(status_code, file_name);

           if(status_code==0)
           {
               //整个过程全部成功
               std::string _stdout;
               FileUtil::ReadFile(PathUtil::Stdout(file_name), &_stdout, true);
               out_value["stdout"]=_stdout;

               std::string _stderr;
               FileUtil::ReadFile(PathUtil::Stderr(file_name), &_stderr, true);
               out_value["stderr"]=_stderr;
           }

           Json::StyledWriter writer;
           *out_json=writer.write(out_value);

           RemoveTempFile(file_name);

        }

    };
}
