#include "Dictonary.h"

// vector<pair<string,int>> _dict;
// map<string,set<int>> _index;
std::shared_ptr<Dictonary> Dictonary::_dictonary = shared_ptr<Dictonary>(new Dictonary());
Dictonary::Dictonary()
:jieba("/home/wyx/interview-prep/project-review/searchs/v5/include/dict/jieba.dict.utf8",
                        "/home/wyx/interview-prep/project-review/searchs/v5/include/dict/hmm_model.utf8",
                        "/home/wyx/interview-prep/project-review/searchs/v5/include/dict/user.dict.utf8",
                        "/home/wyx/interview-prep/project-review/searchs/v5/include/dict/idf.utf8",
                        "/home/wyx/interview-prep/project-review/searchs/v5/include/dict/stop_words.utf8")
{
    std::cout << "字典对象加载中" << std::endl;
    std::ifstream infile1("/home/wyx/interview-prep/project-review/searchs/v5/data/English_fre.txt");
    std::ifstream infile2("/home/wyx/interview-prep/project-review/searchs/v5/data/Chinese_fre.txt");
    std::string line;
    int i = 0;
    vector<pair<string,int>> a;
    vector<pair<string,int>> b;
    while (std::getline(infile1, line)) {
        std::istringstream iss(line);
        std::string word;
        int count;
        iss >> word >> count;
        a.push_back({word, count});
        i++;
    }
    cout << "英文词典加载完成" << "\n";
    _Englishdict = a;
    while (std::getline(infile2, line)) {
        std::istringstream iss(line);
        std::string word;
        int count;
        iss >> word >> count;
        b.push_back({word, count});
    }
    _Chinesedict = b;
    cout << "中午词典加载完成" << "\n";
    std::ifstream infile3("/home/wyx/interview-prep/project-review/searchs/v5/data/EnglishIndex.txt");
    std::ifstream infile4("/home/wyx/interview-prep/project-review/searchs/v5/data/ChineseIndex.txt");

    while (std::getline(infile3, line)) {
        std::istringstream iss(line);
        std::string word;
        std::set<int> intset;
        iss >> word;
        int num;
        while (iss >> num) {
            intset.insert(num);
        }
        _Englishindex[word] = intset;
    }
    cout << "英文索引加载完成" << "\n";
    while (std::getline(infile4, line)) {
        std::istringstream iss(line);
        std::string word;
        std::set<int> intset;
        iss >> word;
        int num;
        while (iss >> num) {
            intset.insert(num);
        }
        _Chineseindex[word] = intset;
    }

    cout << "中文索引加载完成" << "\n";

    infile1.close();
    infile2.close();
    infile3.close();
    infile4.close();

    std::cout << "字典对象加载完成" << std::endl;
}

std::shared_ptr<Dictonary> Dictonary::getInstance() {
    return _dictonary;
}
