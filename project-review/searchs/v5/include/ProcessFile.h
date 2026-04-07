#ifndef __PROCESSFILE_H__
#define __PROCESSFILE_H__
#include <iostream>
#include <fstream>
#include <cctype>
#include <sstream>
#include <map>
#include <string>
#include <filesystem>
#include "cppjieba/Jieba.hpp"
namespace fs = std::filesystem;



class TextProcessor {
public:
    TextProcessor(const std::string& EnglishinputFilePath, const std::string& EnglishoutputFilePath,
    const std::string& ChinesedirectoryPath,const std::string& ChineseoutputFilePath,
    const std::string& Englishstopfile,const std::string& Chinesestopfile);

    void preProcess(cppjieba::Jieba & jieba);
    static size_t nBytesCode(const char ch);
    static int editDistance(const std::string & lhs, const std::string &rhs);
    static size_t length(const std::string &str);
    static int triple_min(const int &a, const int &b, const int &c);

private:
    void EnglishprocessFile();
    void EnglishcountWordOccurrences();
    void EnglishsaveWordFrequencies(const std::map<std::string, int>& wordCount);
    void EnglishprocessIndex(const std::string & filename);
    void ChineseprocessFilesInDirectory(cppjieba::Jieba & jieba);
    void ChinesecleanAndSaveFile(const std::string& inputFilePath, std::ofstream& outputFile);
    void ChinesesaveWordFrequencies(cppjieba::Jieba & jieba);
    void ChineseprocessIndex(const std::string & filename);


private:
    std::string _EnglishinputFilePath;
    std::string _EnglishoutputFilePath;
    std::string _Englishstopfile;

    std::string _ChinesedirectoryPath;
    std::string _ChineseoutputFilePath;
    std::string _Chinesestopfile;
};



#endif

