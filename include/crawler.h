#ifndef CRAWLER_H
#define CRAWLER_H

#include <string>
#include <unordered_set>
#include <vector>

#include "http_client.h"

struct PageInfo {
    std::string url;
    std::string finalUrl;
    std::string parent;
    int depth = 0;
    bool reached = false;
    long statusCode = 0;
    std::string error;
    std::string contentType;
    std::vector<std::string> redirects;
    std::vector<std::string> links;
    double seconds = 0;
};

class Crawler {
public:
    Crawler(int maxPages, int maxDepth);

    void setDelayMs(int ms) { delayMs_ = ms; }
    void run(const std::string& startUrl);

    const std::vector<PageInfo>& pages() const { return pages_; }
    int skippedFiles() const { return skippedFiles_; }
    int leftInQueue() const { return leftInQueue_; }

private:
    PageInfo visit(const std::string& url, int depth, const std::string& parent);
    void printProgress(const PageInfo& page, size_t number) const;

    HttpClient client_;
    int maxPages_;
    int maxDepth_;
    int delayMs_;
    int skippedFiles_;
    int leftInQueue_;
    std::string startUrl_;
    std::vector<PageInfo> pages_;
    std::unordered_set<std::string> visited_;
};

#endif