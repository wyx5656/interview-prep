#include "Echoserver.h"


int main(int argc, char **argv)
{   

    std::cout << "离线部分正在处理 预热中" << std::endl;
    // cppjieba::Jieba jieba("/home/wyx/v5/include/dict/jieba.dict.utf8",
    //                         "/home/wyx/v5/include/dict/hmm_model.utf8",
    //                         "/home/wyx/v5/include/dict/user.dict.utf8",
    //                         "/home/wyx/v5/include/dict/idf.utf8",
    //                         "/home/wyx/v5/include/dict/stop_words.utf8");

    TextProcessor processor("/home/wyx/v5/yuliao/english.txt",
                            "/home/wyx/v5/yuliao/english1.txt",
                            "/home/wyx/v5/yuliao/stop_words_eng.txt",
                            "/home/wyx/v5/yuliao/art/",
                            "/home/wyx/v5/yuliao/chinese1.txt",
                            "/home/wyx/v5/yuliao/stop_words_zh.txt");
    std::shared_ptr<Dictonary> d = Dictonary::getInstance();

    processor.preProcess(d.get()->jieba);
    std::cout << "文件处理完成！" << std::endl;
    RSS rss;
    rss.readFromDirectory("/home/wyx/v5/pageyuliao/");
    rss.store("/home/wyx/v5/data/page1.dat");
    std::cout << "网页清洗完成！" << std::endl;
    webPageHandler::getoffsetlib("/home/wyx/v5/data/page1.dat","/home/wyx/v5/data/offset1.dat");
    webPageHandler::generateIndex(d.get()->jieba);
    Echoserver server(4, 10, "192.168.81.168", 8888);
    server.start();

    return 0;
}

