#include <iostream>
#include <random>
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>

template <class T> using Arr = std::vector<T>;

char buf[1000];
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

int Rand() {
    int r = rng();
    return r < 0 ? -r : r;
}
int Rand(int l, int r) { return Rand() % (r - l + 1) + l; }

int main() {
    // 创建输出文件流
    std::ofstream outfile("test_input.txt");  // 测试点输出文件
    std::ofstream ansfile("test.ans");        // 答案输出文件

    int M = 20;
    Arr<std::string> names(M);
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < 7; ++j)
            names[i] += 'a' + Rand(0, 25);

    Arr<std::string> qry;

    // 使用map来维护数据状态
    std::map<std::string, std::vector<int>> data_map;
    int tot = -1;

    // 生成insert操作
    for (int i = 0; i < 10; ++i) {
        int p = Rand(0, M - 1);
        sprintf(buf, "insert %s %d", names[p].c_str(), ++tot);
        qry.push_back(buf);
        data_map[names[p]].push_back(tot);
    }

    // 生成delete操作
    for (int i = 0; i < 10; ++i) {
        if (Rand(0, 3)) {  // 75%概率删除已有记录
            int p = Rand(0, M - 1);
            if (!data_map[names[p]].empty()) {
                int t = Rand(0, data_map[names[p]].size() - 1);
                sprintf(buf, "delete %s %d", names[p].c_str(), data_map[names[p]][t]);
                qry.push_back(buf);
                data_map[names[p]].erase(data_map[names[p]].begin() + t);
            } else {
                // 如果没有记录可删，改为插入新记录
                sprintf(buf, "insert %s %d", names[p].c_str(), ++tot);
                qry.push_back(buf);
                data_map[names[p]].push_back(tot);
            }
        } else {  // 25%概率尝试删除不存在的记录
            int p = Rand(0, M - 1);
            sprintf(buf, "delete %s %d", names[p].c_str(), ++tot);
            qry.push_back(buf);
            // 不修改map，因为这是一个无效删除
        }
    }

    // 生成find操作并输出答案
    for (int i = 0; i < 10; ++i) {
        int p = Rand(0, M - 1);
        sprintf(buf, "find %s", names[p].c_str());
        qry.push_back(buf);

        // 直接从map中获取答案
        auto& vec = data_map[names[p]];
        if (vec.empty()) {
            ansfile << "null\n";
        } else {
            // 排序输出结果以便于验证
            std::sort(vec.begin(), vec.end());
            for (auto x : vec) {
                ansfile << x << " ";
            }
            ansfile << "\n";
        }
    }

    outfile << qry.size() << "\n";
    for (auto& s : qry) {
        outfile << s << "\n";
    }

    return 0;
}