#include "website_graph.h"
#include "pagerank.h"
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    PageInfo a, b, c;
    a.url = a.finalUrl = "https://test.local/a";
    b.url = b.finalUrl = "https://test.local/b";
    c.url = c.finalUrl = "https://test.local/c";
    a.links = {b.url, c.url};
    b.links = {c.url};
    std::vector<PageInfo> pages = {a, b, c};
    WebsiteGraph graph(pages);
    if (graph.size() != 3 || graph.edgeCount() != 3) return 1;
    if (graph.outDegree(0) != 2 || graph.inDegree(2) != 2) return 1;
    if (graph.orphanPages().size() != 1 || graph.orphanPages()[0] != 0) return 1;
    if (graph.dfs().size() != 3) return 1;
    std::vector<double> rank = calculatePageRank(graph);
    double sum = 0.0;
    for (double value : rank) sum += value;
    if (rank.size() != 3 || std::abs(sum - 1.0) > 1e-6) return 1;
    std::cout << "PASS adjacency list, degrees, DFS, orphan detection, PageRank normalization\n";
    return 0;
}
