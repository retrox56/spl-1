#include "pagerank.h"
#include <cmath>

std::vector<double> calculatePageRank(const WebsiteGraph& graph,
                                      double damping, int maxIterations,
                                      double tolerance) {
    const size_t n = graph.size();
    if (n == 0) return {};
    if (damping < 0.0 || damping >= 1.0) damping = 0.85;
    if (maxIterations < 1) maxIterations = 1;
    if (tolerance < 0.0) tolerance = 1e-10;

    std::vector<double> rank(n, 1.0 / n), next(n, 0.0);
    const auto& edges = graph.adjacency();
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        double danglingMass = 0.0;
        for (size_t u = 0; u < n; ++u)
            if (edges[u].empty()) danglingMass += rank[u];
        const double base = (1.0 - damping) / n + damping * danglingMass / n;
        for (size_t v = 0; v < n; ++v) next[v] = base;
        for (size_t u = 0; u < n; ++u) {
            if (edges[u].empty()) continue;
            const double share = damping * rank[u] / edges[u].size();
            for (int v : edges[u]) next[v] += share;
        }
        double difference = 0.0;
        for (size_t i = 0; i < n; ++i) difference += std::abs(next[i] - rank[i]);
        rank.swap(next);
        if (difference < tolerance) break;
    }
    return rank;
}
