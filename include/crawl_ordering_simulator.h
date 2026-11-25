#ifndef CRAWL_ORDERING_SIMULATOR_H
#define CRAWL_ORDERING_SIMULATOR_H

#include <string>
#include <vector>

struct SimPage {
    std::string url;
    double relevance; // Simulated topic-relevance score in [0, 1]
    std::vector<int> links;
};

struct CrawlRun {
    std::string strategy;
    std::vector<int> visitOrder;
    int importantFound = 0;
    double totalRelevance = 0.0;
    double averageRelevance = 0.0;
};

class CrawlOrderingSimulator {
public:
    explicit CrawlOrderingSimulator(std::vector<SimPage> pages);
    CrawlRun runBfs(int start, int budget, double importantThreshold) const;
    CrawlRun runFocused(int start, int budget, double importantThreshold) const;
    CrawlRun runOpic(int start, int budget, double importantThreshold) const;
    const std::vector<SimPage>& pages() const { return pages_; }

private:
    CrawlRun summarize(const std::string& name, const std::vector<int>& order,
                       double importantThreshold) const;
    std::vector<SimPage> pages_;
};

std::vector<SimPage> makeDemoWebGraph();

#endif
