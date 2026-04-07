#ifndef __WEBPAGEHANDLER_H__
#define __WEBPAGEHANDLER_H__
#include "webPage.h"
#include <cmath>
#include <iostream>
//this define can avoid some logs which you don't need to care about.
#define LOGGER_LEVEL LL_WARN 
#include "./simhash/Simhasher.hpp"
#include "Dictonary.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <unordered_map>
using std::unordered_map;
using std::pair;
using std::vector;
using std::set;
#include <numeric>
#include "cppjieba/Jieba.hpp"
class webPageHandler{
public:
    static void getoffsetlib(const char * path, const char* outputfile);
    static webPage getwebPage(int docid, const char* path);
    static vector<pair<int,pair<int,int>>> offsetlib;
    static unordered_map<string, std::set<std::pair<int, double>>> newnewindex;
    static void removeReputeWeb(const char* path);
    static void generateIndex(cppjieba::Jieba & jieba);
    static bool loadOffsetlib(const char* offset_file);
    static bool loadIndex(const char* index_file);
    static unordered_map<string, std::set<std::pair<int, double>>> normalize(unordered_map<string,set<pair<int,double>>> & newindex);

};


#endif