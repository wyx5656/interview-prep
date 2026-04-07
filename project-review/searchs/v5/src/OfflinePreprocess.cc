#include "ProcessFile.h"
#include "Dictonary.h"
#include "RSS.h"
#include "webPageHandler.h"
#include "cppjieba/Jieba.hpp"

int main(int argc, char **argv)
{
    std::cout << "Starting offline preprocessing..." << std::endl;

    // 预处理中英文词典，生成词频和前缀索引
    TextProcessor processor("/home/wyx/interview-prep/project-review/searchs/v5/yuliao/english.txt",
                            "/home/wyx/interview-prep/project-review/searchs/v5/yuliao/english1.txt",
                            "/home/wyx/interview-prep/project-review/searchs/v5/yuliao/stop_words_eng.txt",
                            "/home/wyx/interview-prep/project-review/searchs/v5/yuliao/art/",
                            "/home/wyx/interview-prep/project-review/searchs/v5/yuliao/chinese1.txt",
                            "/home/wyx/interview-prep/project-review/searchs/v5/yuliao/stop_words_zh.txt");
    std::shared_ptr<Dictonary> d = Dictonary::getInstance();

    processor.preProcess(d.get()->jieba);
    std::cout << "File processing completed!" << std::endl;

    // 清洗RSS网页
    RSS rss;
    rss.readFromDirectory("/home/wyx/interview-prep/project-review/searchs/v5/pageyuliao/");
    rss.store("/home/wyx/interview-prep/project-review/searchs/v5/data/page1.dat");
    std::cout << "Web page cleaning completed!" << std::endl;

    // 生成偏移表，Simhash去重，生成去重后的网页和新偏移表
    webPageHandler::getoffsetlib("/home/wyx/interview-prep/project-review/searchs/v5/data/page1.dat","/home/wyx/interview-prep/project-review/searchs/v5/data/offset1.dat");
    std::cout << "Offset table generated with deduplication, "
              << webPageHandler::offsetlib.size() << " documents remaining" << std::endl;

    // 生成倒排索引和TF-IDF权重
    webPageHandler::generateIndex(d.get()->jieba);
    std::cout << "Inverted index generated successfully!" << std::endl;

    std::cout << "Offline preprocessing completed! All data saved to /home/wyx/interview-prep/project-review/searchs/v5/data/" << std::endl;

    return 0;
}
