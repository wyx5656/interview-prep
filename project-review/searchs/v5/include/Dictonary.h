#ifndef __DICTONARY_H__
#define __DICTONARY_H__
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include "cppjieba/Jieba.hpp"
using namespace std;
class Dictonary {
public:
    static std::shared_ptr<Dictonary> getInstance();
    void setDictonary();
    vector<pair<string,int>> _Englishdict;
    vector<pair<string,int>> _Chinesedict;
    map<string,set<int>> _Englishindex;
    map<string,set<int>> _Chineseindex;
    cppjieba::Jieba jieba;
private:

    Dictonary();// 私有构造函数
    Dictonary(const Dictonary & rhs) = delete;
    Dictonary & operator=(const Dictonary & rhs) = delete;
    static std::shared_ptr<Dictonary> _dictonary;
};
#endif