#include <iostream>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>

void handler(int signo)
{
    std::cout<<"signo: "<<signo<<std::endl;
}
int main()
{
    for(int i=1; i<=31; i++)
    {
        signal(i, handler);
    }

    //测试死循环时间超时
    //struct rlimit r;
    //r.rlim_cur = 1; //设置1秒限制
    //r.rlim_max = RLIM_INFINITY;

    //setrlimit(RLIMIT_CPU, &r);

    //while(1){}
    
    //测试申请内存
    struct rlimit r;
    r.rlim_cur = 1024*1024*40;
    r.rlim_max = RLIM_INFINITY;

    setrlimit(RLIMIT_AS, &r);
    int count=0;
    while(true)
    {
        int *p = new int[1024*1024];
        count++;
        std::cout<<"count: "<<count<<std::endl;
        sleep(1);
    }
    return 0;
}
