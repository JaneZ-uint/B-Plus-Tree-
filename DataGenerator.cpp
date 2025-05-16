//
// Created by JaneZ on 2025/4/30.
//

// Thanks for @coffish 's help.
// I add some file operations so that you can get the output in test_input.txt.
// Answer will be given in test.ans.

#include <iostream>
#include <random>
#include <chrono>
#include <numeric>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

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
#define Q(x, ...) ({ sprintf(buf, x, ##__VA_ARGS__); qry.push_back(buf); })
    Arr<int> perm(M);
    std::iota(perm.begin(), perm.end(), 0);
    std::shuffle(perm.begin(), perm.end(), rng);

    Arr<Arr<int>> ans(M);
    int tot = -1;

    // 生成insert操作
    for (int i = 0; i < 10; ++i) {
        int p = Rand(0, M - 1);
        Q("insert %s %d", names[p].c_str(), ++tot);
        ans[p].push_back(tot);
    }

    // 生成delete操作
    for (int i = 0; i < 10; ++i) {
        if (Rand(0, 3) {  // 75%概率删除已有记录
            int p = Rand(0, M - 1);
            if (!ans[p].empty()) {
                int t = Rand(0, ans[p].size() - 1);
                Q("delete %s %d", names[p].c_str(), ans[p][t]);
                ans[p].erase(ans[p].begin() + t);
            } else {
                // 如果没有记录可删，改为插入新记录
                Q("insert %s %d", names[p].c_str(), ++tot);
                ans[p].push_back(tot);
            }
        } else {  // 25%概率尝试删除不存在的记录
            int p = Rand(0, M - 1);
            Q("delete %s %d", names[p].c_str(), ++tot);
            // 不修改ans数组，因为这是一个无效删除
        }
    }

    // 生成find操作并输出答案
    for (int i = 0; i < 10; ++i) {
        int p = Rand(0, M - 1);
        Q("find %s", names[p].c_str());
        if (ans[p].empty()) {
            ansfile << "null\n";
        } else {
            // 排序输出结果以便于验证
            std::sort(ans[p].begin(), ans[p].end());
            for (auto x : ans[p]) {
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