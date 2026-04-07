#include "webPageHandler.h"
vector<pair<int,pair<int,int>>> webPageHandler::offsetlib;
unordered_map<string, std::set<std::pair<int, double>>> webPageHandler::newnewindex;
void webPageHandler::getoffsetlib(const char * path, const char* outputfile){
    offsetlib.clear();
    std::ifstream infile(path);
    std::ofstream outfile(outputfile);
    if (!infile.is_open()) {
        std::cerr << "无法打开输入文件。" << std::endl;
        return;
    }
    if (!outfile.is_open()) {
        std::cerr << "无法打开输出文件。" << std::endl;
        return;
    }
    std::string line;
    int docid = 1;
    std::streampos currentPosition = infile.tellg();
    while (std::getline(infile, line)) {
        if (line.find("<doc>")!= std::string::npos) {
            std::string docidStr, title, url, content;
            while (std::getline(infile, line) && line.find("<docid>")!= std::string::npos) {
                std::istringstream iss(line.substr(line.find(">") + 1));
                iss >> docidStr;
            }
            while (std::getline(infile, line) && line.find("<title>")!= std::string::npos) {
                std::istringstream iss(line.substr(line.find(">") + 1));
                std::getline(iss, title, '<');
            }
            while (std::getline(infile, line) && line.find("<link>")!= std::string::npos) {
                std::istringstream iss(line.substr(line.find(">") + 1));
                std::getline(iss, url, '<');
            }
            while (std::getline(infile, line) && line.find("<content>")!= std::string::npos) {
                std::istringstream iss(line.substr(line.find(">") + 1));
                std::getline(iss, content, '<');
            }

            std::streampos endPosition = infile.tellg();
            std::streamsize length = endPosition - currentPosition;

            outfile << docid << " " << currentPosition << " " << length << std::endl;
            offsetlib.push_back({docid,{currentPosition,length}});
            docid++;
            currentPosition = endPosition;
        }
    }

    infile.close();
    outfile.close();
    std::cout << "索引文件已经生成" << "\n";
    webPageHandler::removeReputeWeb(path);
}

webPage webPageHandler::getwebPage(int docid,const char* path){
    pair<int,pair<int,int>> off = webPageHandler::offsetlib[docid - 1];
    int first = off.first;
    int offseekg = off.second.first;
    int offlength = off.second.second;

    std::ifstream infile(path);
    if (!infile.is_open()) {
        std::cerr << "无法打开输入文件。" << std::endl;
    }
    infile.seekg(offseekg);
    // 定义一个缓冲区来存储读取的数据
    // 使用动态分配vector代替栈上变长数组，避免栈溢出，符合标准C++
    std::vector<char> buffer(offlength + 1);
    // 从文件中读取数据到缓冲区
    infile.read(buffer.data(), offlength);
    buffer[infile.gcount()] = '\0';// 在读取的内容末尾添加字符串结束符
    infile.close();

    std::string line;
    std::string docidStr, title, url, content;
    std::istringstream iss(buffer.data());
    bool foundDocid = false, foundTitle = false, foundLink = false, foundContent = false;
    while (std::getline(iss, line)) {
        if (!foundDocid && line.find("<docid>")!= std::string::npos) {
            std::istringstream issLine(line.substr(line.find(">") + 1));
            issLine >> docidStr;
            foundDocid = true;
        } else if (!foundTitle && line.find("<title>")!= std::string::npos) {
            std::istringstream issLine(line.substr(line.find(">") + 1));
            std::getline(issLine, title, '<');
            foundTitle = true;
        } else if (!foundLink && line.find("<link>")!= std::string::npos) {
            std::istringstream issLine(line.substr(line.find(">") + 1));
            std::getline(issLine, url, '<');
            foundLink = true;
        } else if (!foundContent && line.find("<content>")!= std::string::npos) {
            std::istringstream issLine(line.substr(line.find(">") + 1));
            std::getline(issLine, content, '<');
            foundContent = true;
        }
        if (foundDocid && foundTitle && foundLink && foundContent) {
            break;
        }
    }
    webPage wp((int)docid, title, url, content);
    return wp;
}

void webPageHandler::removeReputeWeb(const char* path){

    simhash::Simhasher simhasher("/home/wyx/interview-prep/project-review/searchs/v5/include/dict/jieba.dict.utf8", "/home/wyx/interview-prep/project-review/searchs/v5/include/dict/hmm_model.utf8", "/home/wyx/interview-prep/project-review/searchs/v5/include/dict/idf.utf8", "/home/wyx/interview-prep/project-review/searchs/v5/include/dict/stop_words.utf8");

    size_t topN = 5;
    uint64_t u64 = 0;

    // 按最高 8bit 分桶，海明距离≤3意味着最高8bit必须相同，所以只比较同桶内
    const int BUCKET_BITS = 8;
    const int NUM_BUCKETS = 1 << BUCKET_BITS; // 256个桶
    vector<vector<pair<int, uint64_t>>> buckets(NUM_BUCKETS);

    for(int i = 1; i < webPageHandler::offsetlib.size() + 1; i++){
        webPage wp = webPageHandler::getwebPage(i, path);
        simhasher.make(wp._docContent, topN, u64);
        // 计算桶编号：最高8bit
        int bucket = (u64 >> (64 - BUCKET_BITS)) % NUM_BUCKETS;
        buckets[bucket].push_back({wp._docid, u64});
    }

    vector<pair<int,uint64_t>> results;
    // 只比较同一个桶内的文档，大大减少比较次数
    for(int bucket = 0; bucket < NUM_BUCKETS; bucket++){
        auto& bucket_list = buckets[bucket];
        for(int i = 0; i < bucket_list.size(); i++){
            bool duplicate = false;
            uint64_t u1 = bucket_list[i].second;
            // 只和前面已经加入结果的文档比较
            for(int j = 0; j < i; j++){
                uint64_t u2 = bucket_list[j].second;
                if(simhash::Simhasher::isEqual(u1, u2)){
                    duplicate = true;
                    break;
                }
            }
            if(!duplicate){
                results.push_back(bucket_list[i]);
            }
        }
    }
    // 打开新的网页文件和索引文件准备写入
    std::ofstream newWebPageFile("/home/wyx/interview-prep/project-review/searchs/v5/data/new_webpages.dat");
    std::ofstream newIndexFile("/home/wyx/interview-prep/project-review/searchs/v5/data/new_offset.dat");
    if (!newWebPageFile.is_open() ||!newIndexFile.is_open()) {
        std::cerr << "无法打开新的文件进行写入。" << std::endl;
        return;
    }

    int newDocId = 1;
    std::streampos newSeekPosition = 0;
    vector<pair<int,pair<int,int>>> newoffsetlib;
    for (const auto& elem : results) {
        // 根据 docid 从旧的偏移库和网页库中读取数据
        int oldDocId;
        int seekPosition;
        int length;
        // offsetlib 已经按 docid 顺序排列，docid 从 1 开始，直接索引访问 O(1)
        auto& entry = webPageHandler::offsetlib[elem.first - 1];
        oldDocId = entry.first;
        seekPosition = entry.second.first;
        length = entry.second.second;

        if (oldDocId == elem.first) {
            std::ifstream oldWebPageFile(path);
            oldWebPageFile.seekg(seekPosition);
            std::string line;
            std::getline(oldWebPageFile, line);
            while (line.find("<docid>") == std::string::npos) {
                std::getline(oldWebPageFile, line);
            }
            std::istringstream iss(line.substr(line.find(">") + 1));
            iss >> oldDocId;
            while (line.find("<title>") == std::string::npos) {
                std::getline(oldWebPageFile, line);
            }
            std::istringstream issTitle(line.substr(line.find(">") + 1));
            std::getline(issTitle, line, '<');
            std::string title = line;
            while (line.find("<link>") == std::string::npos) {
                std::getline(oldWebPageFile, line);
            }
            std::istringstream issUrl(line.substr(line.find(">") + 1));
            std::getline(issUrl, line, '<');
            std::string url = line;
            while (line.find("<content>") == std::string::npos) {
                std::getline(oldWebPageFile, line);
            }
            std::istringstream issContent(line.substr(line.find(">") + 1));
            std::getline(issContent, line, '<');
            std::string content = line;
            // 将数据写入新的网页文件
            newWebPageFile << "<doc>\n\t<docid>" << newDocId << "</docid>\n\t<title>" << title << "</title>\n\t<link>" << url << "</link>\n\t<content>" << content << "</content>\n</doc>\n";

            // 记录新的索引信息
            newIndexFile << newDocId << " " << newSeekPosition << " " << length << std::endl;
            newoffsetlib.push_back({newDocId,{newSeekPosition,length}});
            newSeekPosition = newWebPageFile.tellp();
            newDocId++;
            oldWebPageFile.close();
        }
    }
    webPageHandler::offsetlib.clear();
    webPageHandler::offsetlib = newoffsetlib;
    newWebPageFile.close();
    newIndexFile.close();
    std::cout << "网页去重成功，新的网页和索引已经生成" << "\n";
}

void webPageHandler::generateIndex(cppjieba::Jieba & jieba){
    //1、加载停用词
    //英文
    std::set<std::string> stopWords;
    std::ifstream fileStream1("/home/wyx/interview-prep/project-review/searchs/v5/yuliao/stop_words_eng.txt");
    if (!fileStream1.is_open()) {
        std::cerr << "无法打开停用词文件：" << "/home/wyx/interview-prep/project-review/searchs/v5/yuliao/stop_words_eng.txt" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(fileStream1, line)) {
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            stopWords.insert(word);
        }
    }
    fileStream1.close();
    std::ifstream fileStream("/home/wyx/interview-prep/project-review/searchs/v5/yuliao/stop_words_zh.txt");
    if (!fileStream.is_open()) {
        std::cerr << "无法打开停用词文件：" << "/home/wyx/interview-prep/project-review/searchs/v5/yuliao/stop_words_zh.txt" << std::endl;
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

    //2、遍历所有文章 得到index数组  string  int  double 分别表示单词  id   和在这个文章中出现的次数  已经去停用词index
    unordered_map<string,set<pair<int,int>>> index; //string   int--id  double --在这个文章中的出现次数
    for(int i = 1; i < webPageHandler::offsetlib.size() + 1; i++){
        webPage wp = webPageHandler::getwebPage(i,"/home/wyx/interview-prep/project-review/searchs/v5/data/new_webpages.dat");
        vector<string> cutString;
        map<string,int> NewcutString;
        jieba.Cut(wp._docContent,cutString,true);

        for(auto s : cutString){
            if(stopWords.find(s) == stopWords.end()){
                NewcutString[s]++;
            }
        }
        for (auto pair1 : NewcutString) {
            index[pair1.first].insert(pair(i, pair1.second));
        }
    }
    //3、对每个单词计算权重
    //文档数
    unordered_map<string,set<pair<int,double>>> newindex;
    int N = webPageHandler::offsetlib.size();
    for(auto & elem : index){
        //关键词
        string s = elem.first;
        //在所有文章中的出现次数
        int DF = elem.second.size();
        for(auto & e : elem.second){
            //在当前文章中的出现次数
            int TF = e.second;
            double w = log2(N / (double)(DF + 1) + 1) * TF;
            newindex[s].insert(pair(e.first,w));
        }
    }

    //归一化权重
    webPageHandler::newnewindex = webPageHandler::normalize(newindex);
    std::cout << "倒排索引生成成功" << "\n";

    std::ofstream outFile("/home/wyx/interview-prep/project-review/searchs/v5/data/index_webpages.dat");
    if (outFile.is_open()) {
        for (const auto &entry : webPageHandler::newnewindex) {
            outFile << entry.first << std::endl;
            for (const auto &pair : entry.second) {
                outFile << pair.first << " " << pair.second << std::endl;
            }
        }
        outFile.close();
    } else {
        std::cerr << "Unable to open file for writing." << std::endl;
    }

}

// 归一化函数
unordered_map<string, std::set<std::pair<int, double>>> webPageHandler::normalize(unordered_map<string, std::set<std::pair<int, double>>> &index) {

    unordered_map<int, set<pair<string, double>>> articleWeights;
    //构建 id 对应的  string的权重
    for (auto &entry : index) {
        for (const auto &pair : entry.second) {
            articleWeights[pair.first].insert(std::pair(entry.first,pair.second));
        }
    }

    unordered_map<int, set<pair<string, double>>> newarticleWeights;
    for(auto & elem : articleWeights){
        double dw = 0;
        for(auto & e : elem.second){
            dw += (e.second * e.second);
        }
        dw = std::sqrt(dw);
        for(auto & e : elem.second){
            newarticleWeights[elem.first].insert(std::pair(e.first,e.second/dw));
        }
    }
    unordered_map<string, std::set<std::pair<int, double>>> newindex;
    for (auto &entry : newarticleWeights) {
        for (auto &pair : entry.second) {
            newindex[pair.first].insert(std::make_pair(entry.first, pair.second));
        }
    }

    return newindex;

}

bool webPageHandler::loadOffsetlib(const char* offset_file) {
    std::ifstream infile(offset_file);
    if (!infile.is_open()) {
        std::cerr << "Cannot open offset file: " << offset_file << std::endl;
        return false;
    }

    offsetlib.clear();
    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        int docid, offset, length;
        iss >> docid >> offset >> length;
        offsetlib.push_back({docid, {offset, length}});
    }
    infile.close();
    std::cout << "Loaded " << offsetlib.size() << " document offsets" << std::endl;
    return true;
}

bool webPageHandler::loadIndex(const char* index_file) {
    std::ifstream infile(index_file);
    if (!infile.is_open()) {
        std::cerr << "Cannot open index file: " << index_file << std::endl;
        return false;
    }

    newnewindex.clear();
    std::string current_word;
    std::string line;

    // 读取第一行作为第一个词
    if (!std::getline(infile, current_word)) {
        infile.close();
        std::cout << "Loaded inverted index with " << newnewindex.size() << " terms" << std::endl;
        return false;
    }

    while (!current_word.empty()) {
        if (current_word.empty()) {
            if (!std::getline(infile, current_word)) break;
            continue;
        }

        bool got_next_word = false;
        // 读取所有docid weight行，直到碰到下一个词行（无法解析为两个数字）
        while (std::getline(infile, line)) {
            if (line.empty()) {
                continue; // skip empty lines
            }
            std::istringstream iss(line);
            int docid;
            double weight;
            // 尝试解析为docid和weight，如果成功就是doc行
            if (iss >> docid >> weight) {
                newnewindex[current_word].insert({docid, weight});
            } else {
                // 解析失败，说明这行是下一个词
                current_word = line;
                got_next_word = true;
                break;
            }
        }
        if (!got_next_word) {
            // 文件结束，退出
            break;
        }
    }
    infile.close();
    std::cout << "Loaded inverted index with " << newnewindex.size() << " terms" << std::endl;
    return true;
}