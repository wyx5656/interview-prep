#ifndef __RSS_H__
#define __RSS_H__

#include "tinyxml2.h"
#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <fstream>
#include <filesystem>
#include "RSSITem.h"
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ofstream;
using namespace tinyxml2;
using namespace std::filesystem;


class RSS {
public:
    RSS();

    void readFromDirectory(const string& directoryPath);
    void readSingleFile(const string& filename);
    void store(const string& filename);

private:
    vector<RSSItem> _rss;
};

#endif
