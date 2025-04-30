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
    for (int i = 0; i < 1000; ++i) {  //can be changed
        int p = Rand(0, M - 1);
        Q("insert %s %d", names[p].c_str(), ++tot);
        ans[p].push_back(tot);
    }

    for (int i = 0; i < 1000; ++i) {  //can be changed
        if (Rand(0, 3) < 3) {
            int p = Rand(0, M - 1);
            Q("delete %s %d", names[p].c_str(), ++tot);
            ans[p].push_back(tot);
        } else {
            int p = Rand(0, M - 1);
            int ra = ans[p].size();
            if (ra > 0) {
                int t = Rand(0, ra - 1);
                Q("delete %s %d", names[p].c_str(), ans[p][t]);
                ans[p].erase(ans[p].begin() + t);
            } else {
                --i;
            }
        }
    }

    for (int i = 0; i < 1000; ++i) {  //can be changed
        int p = Rand(0, M - 1);
        Q("find %s", names[p].c_str());
        if (ans[p].empty())
            ansfile << "null\n";
        else {
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