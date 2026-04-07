#include "tinyxml2.h"
#include "RSS.h"
#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <fstream>
#include <filesystem>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ofstream;
using namespace tinyxml2;
using namespace std::filesystem;

RSS::RSS() {}

void RSS::readFromDirectory(const string& directoryPath) {
    int totalFileCount = 0;
    int processedFileCount = 0;
    for (const auto& entry : directory_iterator(directoryPath)) {
        if (is_regular_file(entry) && entry.path().extension() == ".xml") {
            totalFileCount++;
            readSingleFile(entry.path().string());
            processedFileCount++;
        }
    }
    cout << "Total XML files found: " << totalFileCount << endl;
    cout << "Processed XML files: " << processedFileCount << endl;
}

void RSS::readSingleFile(const string& filename) {
    XMLDocument doc;
    XMLError eResult = doc.LoadFile(filename.c_str());
    if (eResult!= XML_SUCCESS) {
        std::cerr << "loadFile " << filename << " fail" << endl;
        return;
    }

    XMLElement* rssNode = doc.FirstChildElement("rss");
    if (!rssNode) {
        std::cerr << "No <rss> element found in " << filename << "." << endl;
        return;
    }

    XMLElement* channelNode = rssNode->FirstChildElement("channel");
    if (!channelNode) {
        std::cerr << "No <channel> element found under <rss> in " << filename << "." << endl;
        return;
    }

    XMLElement* itemNode = channelNode->FirstChildElement("item");
    while (itemNode) {
        RSSItem rssItem;
        bool hasTitle = false, hasLink = false, hasContent = false;

        XMLElement* titleNode = itemNode->FirstChildElement("title");
        if (titleNode && titleNode->GetText() && titleNode->GetText()[0]!= '\0') {
            rssItem.title = titleNode->GetText();
            hasTitle = true;
        }
        else {
            std::cerr << "Missing or empty <title> element found under <item> in " << filename << ". Skipping this item." << endl;
            itemNode = itemNode->NextSiblingElement("item");
            continue;
        }

        XMLElement* linkNode = itemNode->FirstChildElement("link");
        if (linkNode && linkNode->GetText() && linkNode->GetText()[0]!= '\0') {
            rssItem.link = linkNode->GetText();
            hasLink = true;
        }
        else {
            std::cerr << "Missing or empty <link> element found under <item> in " << filename << ". Skipping this item." << endl;
            itemNode = itemNode->NextSiblingElement("item");
            continue;
        }

        // Try to get <content:encoded> first, fall back to <description>
        XMLElement* contentNode = itemNode->FirstChildElement("content:encoded");
        if (!contentNode) {
            contentNode = itemNode->FirstChildElement("description");
        }

        if (contentNode && contentNode->GetText() && contentNode->GetText()[0]!= '\0') {
            rssItem.content = contentNode->GetText();
            hasContent = true;
        }
        else {
            std::cerr << "Both <content:encoded> and <description> elements missing or empty under <item> in " << filename << ". Skipping this item." << endl;
            itemNode = itemNode->NextSiblingElement("item");
            continue;
        }

        // Check if all required fields are present and not empty before adding to the list
        if (hasTitle && hasLink && hasContent) {
            std::regex reg("<[^>]+>");
            rssItem.content = std::regex_replace(rssItem.content, reg, "");
            _rss.push_back(rssItem);
        }

        itemNode = itemNode->NextSiblingElement("item");
    }
}

void RSS::store(const string& filename) {
    ofstream ofs(filename);
    if (!ofs.is_open()) {
        std::cerr << "open " << filename << " fail!" << endl;
        return;
    }

    for (size_t idx = 0; idx < _rss.size(); ++idx) {
        ofs << "<doc>\n\t<docid>" << idx + 1
            << "</docid>\n\t<title>" << _rss[idx].title
            << "</title>\n\t<link>" << _rss[idx].link
            << "</link>\n\t<content>" << _rss[idx].content << "</content>\n</doc>\n";
    }

    ofs.close();
}


