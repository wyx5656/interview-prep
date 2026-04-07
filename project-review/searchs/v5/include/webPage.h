#ifndef __WEBPAGE_H__
#define __WEBPAGE_H__
#include <string>
#include <map>
using std::string;
using std::map;
class webPage{
public:
    int _docid;
    string _docTitle;
    string _docUrl;
    string _docContent;
    webPage(int docid,string docTitle,string docUrl,string docContent);
};

#endif
