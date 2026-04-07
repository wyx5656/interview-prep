#include "cppjieba/Jieba.hpp"
#include "string"
using namespace std;

const char* DICT_PATH="../include/dict/jieba.dict.utf8";
const char* HMM_PATH="../include/dict/hmm_model.utf8";
const char* USER_DICT_PATH="../include/dict/user.dict.utf8";
const char* IDF_PATH = "../include/dict/idf.utf8";
const char* STOP_WORD_PATH="../include/dict/stop_words.utf8";

int main(int argc,char**argv){

    cppjieba::Jieba jieba(DICT_PATH,HMM_PATH,USER_DICT_PATH,IDF_PATH,STOP_WORD_PATH);
    vector<string> words;
    string s = "他来到了网易大厦\nasdasdasda";
    jieba.Cut(s,words,true);
    for(auto word:words){
        cout << "word = " << word << "\n";
    }

}