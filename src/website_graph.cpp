#include "website_graph.h"
#include <algorithm>
#include <functional>
#include <unordered_map>

WebsiteGraph::WebsiteGraph(const std::vector<PageInfo>& pages) {
    std::unordered_map<std::string, int> index;
    for (const PageInfo& page : pages) {
        const std::string& key = page.finalUrl.empty() ? page.url : page.finalUrl;
        if (index.find(key) == index.end()) {
            int id = static_cast<int>(urls_.size());
            index[key] = id;
            urls_.push_back(key);
        }
    }
    adjacency_.resize(urls_.size());
    indegree_.assign(urls_.size(), 0);
    for (const PageInfo& page : pages) {
        const std::string& source = page.finalUrl.empty() ? page.url : page.finalUrl;
        auto from = index.find(source);
        if (from == index.end()) continue;
        for (const std::string& link : page.links) {
            auto to = index.find(link);
            if (to == index.end()) continue; // only edges to pages actually crawled
            auto& edges = adjacency_[from->second];
            if (std::find(edges.begin(), edges.end(), to->second) == edges.end()) {
                edges.push_back(to->second);
                ++indegree_[to->second];
                ++edgeCount_;
            }
        }
    }
}

int WebsiteGraph::inDegree(int vertex) const {
    return vertex >= 0 && static_cast<size_t>(vertex) < indegree_.size() ? indegree_[vertex] : 0;
}

int WebsiteGraph::outDegree(int vertex) const {
    return vertex >= 0 && static_cast<size_t>(vertex) < adjacency_.size()
        ? static_cast<int>(adjacency_[vertex].size()) : 0;
}

std::vector<int> WebsiteGraph::dfs(int start) const {
    std::vector<int> order;
    if (start < 0 || static_cast<size_t>(start) >= urls_.size()) return order;
    std::vector<bool> visited(urls_.size(), false);
    std::function<void(int)> visit = [&](int u) {
        visited[u] = true;
        order.push_back(u);
        for (int v : adjacency_[u]) if (!visited[v]) visit(v);
    };
    visit(start);
    return order;
}

std::vector<int> WebsiteGraph::orphanPages() const {
    std::vector<int> result;
    for (size_t i = 0; i < indegree_.size(); ++i)
        if (indegree_[i] == 0) result.push_back(static_cast<int>(i));
    return result;
}
