
#include"ProcessFile.h"

TextProcessor::TextProcessor(const std::string& EnglishinputFilePath, const std::string& EnglishoutputFilePath,
const std::string& Englishstopfile,const std::string& ChinesedirectoryPath,const std::string& ChineseoutputFilePath,
const std::string& Chinesestopfile)
: _EnglishinputFilePath(EnglishinputFilePath)
, _EnglishoutputFilePath(EnglishoutputFilePath)
,_Englishstopfile(Englishstopfile)
,_ChinesedirectoryPath(ChinesedirectoryPath) 
,_ChineseoutputFilePath(ChineseoutputFilePath)
,_Chinesestopfile(Chinesestopfile)
{}

void TextProcessor::preProcess(cppjieba::Jieba & jieba){
    EnglishprocessFile();
    ChineseprocessFilesInDirectory(jieba);
}

void TextProcessor::EnglishprocessFile(){
    std::ifstream inputFile(_EnglishinputFilePath);// ~/v5/yuliao/english.txt
    if (!inputFile.is_open()) {
        std::cerr << "无法打开输入文件：" << _EnglishinputFilePath << std::endl;
        return;
    }

    std::ofstream outputFile(_EnglishoutputFilePath);
    if (!outputFile.is_open()) {
        std::cerr << "无法创建输出文件：" << _EnglishoutputFilePath << std::endl;
        inputFile.close();
        return;
    }

    char ch;
    while (inputFile.get(ch)) { // 逐字符读取文件
        if (std::isalpha(ch)) { // 如果是字母，转换为小写
            outputFile.put(std::tolower(ch));
        } else if (std::isdigit(ch)) { // 如果是数字，直接输出
            outputFile.put(ch);
        } else { // 否则，转换为空格
            outputFile.put(' ');
        }
    }

    inputFile.close();
    outputFile.close();
    EnglishcountWordOccurrences();
}

void TextProcessor::EnglishcountWordOccurrences(){
    std::ifstream fileStream(_EnglishoutputFilePath);
    if (!fileStream.is_open()) {
        std::cerr << "无法打开文件：" << _EnglishoutputFilePath << std::endl;
        return;
    }

    std::string word;
    std::map<std::string, int> wordCount;

    // 逐词读取文件
    while (fileStream >> word) {
        ++wordCount[word];
    }
    EnglishsaveWordFrequencies(wordCount);
    // 输出结果
    fileStream.close();

}

void TextProcessor::EnglishsaveWordFrequencies(const std::map<std::string, int>& wordCount){
    //停用词
    std::set<std::string> stopWords;
    std::ifstream fileStream(_Englishstopfile);// ~/v5/yuliao/stop_words_eng.txt
    if (!fileStream.is_open()) {
        std::cerr << "无法打开停用词文件：" << _Englishstopfile << std::endl;
        return;
    }

    std::string line;
    while (std::getline(fileStream, line)) {
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            stopWords.insert(word);
        }
    }

    std::ofstream outputFile("/home/wyx/interview-prep/project-review/searchs/v5/data/English_fre.txt");
    if (!outputFile.is_open()) {
        std::cerr << "无法创建输出文件：" << "English_fre.txt" << std::endl;
        return;
    }

    // 输出结果
    for (const auto& pair : wordCount) {
        if (stopWords.find(pair.first) == stopWords.end()) {
            outputFile << pair.first << " " << pair.second << std::endl;
        }
    }
    fileStream.close();
    outputFile.close();
    EnglishprocessIndex("/home/wyx/interview-prep/project-review/searchs/v5/data/English_fre.txt");
}

void TextProcessor::EnglishprocessIndex(const std::string & filename){
    //1、读取字典文件，生成单词和索引
    std::vector<std::pair<std::string, int>> dict;
    std::ifstream infile(filename);
    std::string line;
    int index = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string word;
        int count;
        iss >> word >> count;
        dict.push_back({word, index});
        index++;
    }
    //2、单词索引生成字母索引的映射
    std::map<std::string, std::set<int>> indexMap;
    for (const auto& pair : dict) {
        std::string word = pair.first;
        for (char c : word) {
            std::string letter = std::string(1, c);
            indexMap[letter].insert(pair.second);
        }
    }

    std::ofstream outfile("/home/wyx/interview-prep/project-review/searchs/v5/data/EnglishIndex.txt");
    for (const auto& pair : indexMap) {
        outfile << pair.first << " ";
        for (int index : pair.second) {
            outfile << index << " ";
        }
        outfile << std::endl;
    }

}

void TextProcessor::ChineseprocessFilesInDirectory(cppjieba::Jieba & jieba){
    std::ofstream outputFile(_ChineseoutputFilePath, std::ios::out | std::ios::trunc);
    if (!outputFile.is_open()) {
        std::cerr << "无法创建输出文件：" << _ChineseoutputFilePath << std::endl;
        return;
    }
    for (const auto& entry : fs::directory_iterator(_ChinesedirectoryPath)) {//~/v5/yuliao/art/
        if (entry.path().extension() == ".txt") {
            std::string inputFilePath = entry.path().string();
            std::string outputFilePath = inputFilePath.substr(0, inputFilePath.find_last_of('.')) + "_cleaned.txt";

            ChinesecleanAndSaveFile(inputFilePath, outputFile);
        }
    }

    outputFile.close();
    ChinesesaveWordFrequencies(jieba);

}

void TextProcessor::ChinesecleanAndSaveFile(const std::string& inputFilePath, std::ofstream& outputFile){
    std::ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "无法打开输入文件：" << inputFilePath << std::endl;
        return;
    }


    char ch;
    while (inputFile.get(ch)) { // 逐字符读取文件
        if (ch != '\r' && ch != '\n') { // 如果不是换行符或回车符
            outputFile.put(ch);
        }
    }

    inputFile.close();
}

void TextProcessor::ChinesesaveWordFrequencies(cppjieba::Jieba & jieba){
    std::ifstream inputFile(_ChineseoutputFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "无法打开输入文件：" << _ChineseoutputFilePath << std::endl;
        return;
    }
    std::map<std::string, int> wordCount;

    std::string line;
    int i = 0;
    while (std::getline(inputFile, line)) {
        std::vector<std::string> words;
        jieba.Cut(line, words, true); // 使用精确模式分词
        // 统计词频
        for (const auto& word : words) {
            if (!word.empty()) {
                ++wordCount[word];
            }
        }
    }
    inputFile.close();



    //停用词
    std::set<std::string> stopWords;
    std::ifstream fileStream(_Chinesestopfile);
    if (!fileStream.is_open()) {
        std::cerr << "无法打开停用词文件：" << _Chinesestopfile << std::endl;
        return;
    }

    std::string line1;
    while (std::getline(fileStream, line1)) {
        std::istringstream iss(line1);
        std::string word;
        while (iss >> word) {
            stopWords.insert(word);
        }
    }

    // 保存结果到文件
    std::ofstream outputFile("/home/wyx/interview-prep/project-review/searchs/v5/data/Chinese_fre.txt");
    if (!outputFile.is_open()) {
        std::cerr << "无法创建输出文件：" << "Chinese_fre.txt" << std::endl;
        return;
    }

    // 输出结果
    for (const auto& pair : wordCount) {
        if(stopWords.find(pair.first) == stopWords.end()){
            outputFile << pair.first << " " << pair.second << std::endl;
        }
    }
    outputFile.close();
    ChineseprocessIndex("/home/wyx/interview-prep/project-review/searchs/v5/data/Chinese_fre.txt");
}

void TextProcessor::ChineseprocessIndex(const std::string & filename){
    //1、读取字典文件，生成单词和索引
    std::vector<std::pair<std::string, int>> dict;
    std::ifstream infile(filename);
    std::string line;
    int index = 0;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string word;
        int count;
        iss >> word >> count;
        dict.push_back({word, index});
        index++;
    }
    //2、单词索引生成字母索引的映射
    std::map<std::string, std::set<int>> indexMap;
    for (const auto& pair : dict) {
        std::string word = pair.first;
        size_t pos = 0;
        while (pos < word.length()) {
            size_t nBytes = nBytesCode(word[pos]);
            std::string letter(word.substr(pos, nBytes));
            indexMap[letter].insert(pair.second);
            pos += nBytes;
        }
    }

    std::ofstream outfile("/home/wyx/interview-prep/project-review/searchs/v5/data/ChineseIndex.txt");
    for (const auto& pair : indexMap) {
        outfile << pair.first << " ";
        for (int index : pair.second) {
            outfile << index << " ";
        }
        outfile << std::endl;
    }
}

size_t TextProcessor::nBytesCode(const char ch){
    if(ch & (1 << 7)){
        int nBytes = 1;
        for(int idx = 0; idx != 6; ++idx){
            if(ch & (1 << (6 - idx))){
                ++nBytes;
            }else{
                break;
            }
        }
        return nBytes;
    }
    return 1;
}

int TextProcessor::editDistance(const std::string & lhs, const std::string &rhs)
{
    //计算最小编辑距离-包括处理中英文
    size_t lhs_len = length(lhs);
    size_t rhs_len = length(rhs);
    // 动态分配二维数组，避免栈溢出（原代码栈上变长数组是C99特性，C++不标准）
    std::vector<std::vector<int>> editDist(lhs_len + 1);
    for(size_t idx = 0; idx <= lhs_len; ++idx) {
        editDist[idx].resize(rhs_len + 1);
        editDist[idx][0] = idx;
    }

    for(size_t idx = 0; idx <= rhs_len; ++idx) {
        editDist[0][idx] = idx;
    }

    std::string sublhs, subrhs;
    for(std::size_t dist_i = 1, lhs_idx = 0; dist_i <= lhs_len; ++dist_i,++lhs_idx)
    {
        size_t nBytes = nBytesCode(lhs[lhs_idx]);
        sublhs = lhs.substr(lhs_idx, nBytes);
        lhs_idx += (nBytes - 1);
        for(std::size_t dist_j = 1, rhs_idx = 0;dist_j <= rhs_len; ++dist_j, ++rhs_idx)
        {
            nBytes = nBytesCode(rhs[rhs_idx]);
            subrhs = rhs.substr(rhs_idx, nBytes);
            rhs_idx += (nBytes - 1);
            if(sublhs == subrhs)
            {
                editDist[dist_i][dist_j] = editDist[dist_i - 1][dist_j -1];
            }
            else
            {
                editDist[dist_i][dist_j] =
                triple_min(editDist[dist_i][dist_j - 1] + 1,
                editDist[dist_i - 1][dist_j] + 1,
                editDist[dist_i - 1][dist_j - 1] + 1);
            }
        }
    }
    return editDist[lhs_len][rhs_len];
}

std::size_t TextProcessor::length(const std::string &str)
{
    std::size_t ilen = 0;
    for(std::size_t idx = 0; idx != str.size(); ++idx)
    {
        int nBytes = nBytesCode(str[idx]);
        idx += (nBytes - 1);
        ++ilen;
    }
    return ilen;
}

int TextProcessor::triple_min(const int &a, const int &b, const int &c)
{
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}
