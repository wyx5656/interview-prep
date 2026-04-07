#include "webPageHandler.h"


int main(){
    webPageHandler::getoffsetlib("pagelib.dat", "a.dat");
    for(auto & s : webPageHandler::offsetlib){
        std::cout << s.first <<" " << s.second.first << " " << s.second.second << " " << std::endl;
    }

    webPage X = webPageHandler::getwebPage(2,"pagelib.dat");
    webPageHandler::removeReputeWeb("pagelib.dat");
}