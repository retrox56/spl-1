#ifndef WEBSITE_GRAPH_H
#define WEBSITE_GRAPH_H

#include "crawler.h"
#include <string>
#include <vector>

class WebsiteGraph {
public:
    explicit WebsiteGraph(const std::vector<PageInfo>& pages);
    size_t size() const { return urls_.size(); }
    const std::vector<std::string>& urls() const { return urls_; }
    const std::vector<std::vector<int>>& adjacency() const { return adjacency_; }
    int inDegree(int vertex) const;
    int outDegree(int vertex) const;
    std::vector<int> dfs(int start = 0) const;
    std::vector<int> orphanPages() const;
    size_t edgeCount() const { return edgeCount_; }

private:
    std::vector<std::string> urls_;
    std::vector<std::vector<int>> adjacency_;
    std::vector<int> indegree_;
    size_t edgeCount_ = 0;
};

#endif
