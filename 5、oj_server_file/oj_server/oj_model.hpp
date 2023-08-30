#pragma once
#include "../common/util.hpp"
#include "../common/log.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <cassert>

// 根据题目list文件，加载所有的题目信息到内存中
// model: 主要用来和数据进行交互，对外提供访问数据的接口

namespace ns_model
{
    using namespace std;
    using namespace ns_log;
    using namespace ns_util;

    struct Question
    {
        string number; // 题目编号，唯一
        string title;  // 题目的标题
        string star;   // 难度：简单，中等，困难
        int cpu_limit; // 题目的时间要求 s
        int mem_limit; // 题目的空间要求 kb
        string desc;   // 题目的描述
        string header; // 题目预设给用户在线编辑器的代码
        string tail;   // 题目的测试用例， 需要和header拼接，形成完整代码
    };

    const string ques_list = "./questions/questions.list";
    const string ques_path = "./questions/";

    class Model
    {
    private:
        // 题目和题目细节的对应
        unordered_map<string, Question> questions;

    public:
        Model()
        {
            assert(LoadQuestionList(ques_list));
        }
        bool LoadQuestionList(const string &questions_list)
        {
            // 加载配置文件: questions/questions.list + 题目编号文件
            ifstream in(questions_list);
            if (!in.is_open())
            {
                LOG(FATAL) << "加载题库失败，请检查是否存在题库文件"
                           << "\n";
                return false;
            }

            string line;
            while (getline(in, line))
            {
                vector<string> tokens;
                StringUtil::SplitString(line, &tokens, " ");
                // 1 判断回文数 简单 1 30000
                // 每一行5个部分，读取成功应该有5个
                if (tokens.size() != 5)
                {
                    LOG(WARNING) << "加载部分题目失败，请检查文件格式"
                                 << "\n";
                    continue;
                }

                Question q;
                q.number = tokens[0];
                q.title = tokens[1];
                q.star = tokens[2];
                q.cpu_limit = atoi(tokens[3].c_str());
                q.mem_limit = atoi(tokens[4].c_str());

                string path = ques_path;
                path += q.number;
                path += "/";

                FileUtil::ReadFile(path + "desc.txt", &(q.desc), true);
                FileUtil::ReadFile(path + "header.cpp", &(q.header), true);
                FileUtil::ReadFile(path + "tail.cpp", &(q.tail), true);

                questions.insert({q.number, q});
            }

            LOG(INFO) << "加载题库成功"
                      << "\n";
            in.close();
            return true;
        }

        bool GetAllQuestions(vector<Question> *out)
        {
            if (questions.size() == 0)
            {
                LOG(ERROR) << "用户获取题库失败"
                           << "\n";
                return false;
            }

            for (const auto &q : questions)
            {
                out->push_back(q.second);
            }
            return true;
        }

        bool GetOneQuestion(const string &number, Question *q)
        {
            const auto &iter = questions.find(number);
            if (iter == questions.end())
            {
                LOG(ERROR) << "用户获取题目失败，题目编号：" << number << "\n";
                return false;
            }
            (*q) = iter->second;
            return true;
        }
        ~Model() {}
    };
}
