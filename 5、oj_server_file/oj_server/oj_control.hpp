#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cassert>
#include <mutex>
#include <algorithm>
#include <jsoncpp/json/json.h>

#include "../common/log.hpp"
#include "../common/util.hpp"
#include "../common/httplib.h"

#include "oj_model.hpp"
#include "oj_view.hpp"

namespace ns_control
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_model;
    using namespace ns_view;
    using namespace httplib;

    // 提供服务的主机
    class Machine
    {
    public:
        std::string ip;  // 编译服务的IP
        int port;        // 编译服务的port
        uint64_t load;   // 编译服务的负载
        std::mutex *mtx; // mutex禁止拷贝，使用指针
    public:
        Machine() : ip(""), port(0), load(0), mtx(nullptr)
        {
        }
        ~Machine() {}

    public:
        // 减少主机负载
        void DecLoad()
        {
            if (mtx)
                mtx->lock();
            --load;
            if (mtx)
                mtx->unlock();
        }

        // 提升主机负载
        void IncLoad()
        {
            if (mtx)
                mtx->lock();
            ++load;
            if (mtx)
                mtx->unlock();
        }

        void ResetLoad()
        {
            if (mtx)
                mtx->lock();
            load = 0;
            if (mtx)
                mtx->unlock();
        }

        // 获取主机负载
        uint64_t Load()
        {
            uint64_t _load = 0;
            if (mtx)
                mtx->lock();
            _load = load;
            if (mtx)
                mtx->unlock();
            return _load;
        }
    };

    const std::string service_machine = "./conf/service_machine.conf";

    // 负载均衡模块
    class LoadBlance
    {
    private:
        std::vector<Machine> machines; // 提升编译服务的主机，每一台主机都有自己的下标，充当当前主机的ID
        // 所有在线的主机id
        std::vector<int> online;
        // 所有离线主机的id
        std::vector<int> offline;

        // 锁 保证loadblance数据安全
        std::mutex mtx;

    public:
        LoadBlance()
        {
            assert(LoadConf(service_machine));
            LOG(INFO) << "加载 " << service_machine << " 成功"
                      << "\n";
        }
        ~LoadBlance() {}

    public:
        bool LoadConf(const std::string &machine_conf)
        {
            std::ifstream in(machine_conf);
            if (!in.is_open())
            {
                LOG(FATAL) << " 加载" << machine_conf << " 失败"
                           << "\n";
                return false;
            }

            std::string line;
            while (std::getline(in, line))
            {
                std::vector<std::string> tokens;
                StringUtil::SplitString(line, &tokens, ":");
                if (tokens.size() != 2)
                {
                    LOG(WARNING) << " 切分" << line << " 失败"
                                 << "\n";
                    continue;
                }

                Machine m;
                m.ip = tokens[0];
                m.port = atoi(tokens[1].c_str());
                m.load = 0;
                m.mtx = new std::mutex();

                online.push_back(machines.size());
                machines.push_back(m);
            }

            in.close();
            return true;
        }

        bool SmartChoice(int *id, Machine **m)
        {
            mtx.lock();

            int online_num = online.size();
            if (online_num == 0)
            {
                mtx.unlock();
                LOG(FATAL) << " 所有后端编译主机已经离线，请运维的同事尽快查看"
                           << "\n";
                return false;
            }

            // 通过遍历方式找到负载最小的主机
            *id = online[0];
            *m = &machines[online[0]];
            uint64_t min_load = machines[online[0]].Load();
            for (int i = 1; i < online_num; i++)
            {
                uint64_t cur_load = machines[online[i]].Load();
                if (min_load > cur_load)
                {
                    min_load = cur_load;
                    *id = online[i];
                    *m = &machines[online[i]];
                }
            }

            mtx.unlock();
            return true;
        }

        void OfflineMachine(int which)
        {
            mtx.lock();
            for (auto iter = online.begin(); iter != online.end(); iter++)
            {
                if (*iter == which)
                {
                    machines[which].ResetLoad();
                    online.erase(iter);
                    offline.push_back(which);
                    break;
                }
            }

            mtx.unlock();
        }

        void OnlineMachine()
        {
            mtx.lock();
            online.insert(online.end(), offline.begin(), offline.end());
            offline.erase(offline.begin(), offline.end());

            mtx.unlock();
            LOG(INFO) << "所有主机上线了"
                      << "\n";
        }

        void ShowMachines()
        {
            mtx.lock();
            std::cout << "当前在线主机列表: ";

            for (auto &id : online)
            {
                std::cout << id << " ";
            }

            std::cout << std::endl;
            std::cout << "当前离线主机列表: ";

            for (auto &id : offline)
            {
                std::cout << id << " ";
            }

            std::cout << std::endl;

            mtx.unlock();
        }
    };

    class Control
    {
    private:
        Model model_;            // 提供后台数据
        View view_;              // 提供html渲染功能
        LoadBlance load_blance_; // 负载均衡器

    public:
        Control() {}
        ~Control() {}

    public:
        void RecoveryMachine()
        {
            load_blance_.OnlineMachine();
        }

        // 根据题目数据构建网页
        //  html: 输出形参数
        bool AllQuestions(string *html)
        {
            bool ret = true;
            vector<struct Question> all;
            if (model_.GetAllQuestions(&all))
            {
                sort(all.begin(), all.end(), [](const struct Question &q1, const struct Question &q2)
                     { return atoi(q1.number.c_str()) < atoi(q2.number.c_str()); });
                // 获取题目信息成功，将所有题目数据构建成网页
                view_.AllExpandHtml(all, html);
            }
            else
            {
                *html = "获取题目失败, 形成题目列表失败";
                ret = false;
            }
            return ret;
        }

        bool Question(const string &number, string *html)
        {
            bool ret = true;
            struct Question q;
            if (model_.GetOneQuestion(number, &q))
            {
                // 获取指定题目的信息成功，将指定题目数据构建成网页
                view_.OneExpandHtml(q, html);
            }
            else
            {
                *html = "指定题目：" + number + "不存在！";
                ret = false;
            }
            return ret;
        }

        void Judge(const std::string &number, const std::string in_json, std::string *out_json)
        {
            // 根据题号，拿到对应题目细节
            struct Question q;
            model_.GetOneQuestion(number, &q);

            // injson 进行反序列化，得到题目的id, 得到用户提交的源代码和Input
            Json::Reader reader;
            Json::Value in_value;
            reader.parse(in_json, in_value);

            std::string code = in_value["code"].asString();

            // 重新拼接用户代码和测试用例代码，形成新的代码
            Json::Value compile_value;
            compile_value["input"] = in_value["input"].asString();
            compile_value["code"] = code + "\n" + q.tail;
            compile_value["cpu_limit"] = q.cpu_limit;
            compile_value["mem_limit"] = q.mem_limit;

            Json::FastWriter writer;
            std::string compile_string = writer.write(compile_value);

            // 选择负载最低的主机   一直进行选择，直到有主机可用
            while (true)
            {
                int id = 0;
                Machine *m = nullptr;
                if (!load_blance_.SmartChoice(&id, &m))
                {
                    break;
                }

                // 选择成功，发起http请求，得到结果
                Client cli(m->ip, m->port);
                m->IncLoad();

                LOG(INFO) << "选择主机成功，主机id: " << id << "详情： " << m->ip << ":" << m->port << " 当前主机的负载是：" << m->Load() << "\n";
                if (auto res = cli.Post("/compile_and_run", compile_string, "application/json;charset=utf-8"))
                {
                    // 将结果赋给out_json
                    if (res->status == 200)
                    {
                        *out_json = res->body;
                        m->DecLoad();
                        LOG(INFO) << "请求编译和运行服务成功..."
                                  << "\n";
                        break;
                    }

                    m->DecLoad();
                }
                else
                {
                    LOG(ERROR) << " 当前请求的主机id: " << id << " 详情" << m->ip << ":" << m->port << " 可能已经离线"
                               << "\n";
                    load_blance_.OfflineMachine(id);
                    load_blance_.ShowMachines(); // 仅仅用来调试
                }
            }
        }
    };
}