#include "crawl_ordering_simulator.h"

#include <algorithm>
#include <queue>
#include <utility>
#include <stdexcept>
#include <unordered_set>

CrawlOrderingSimulator::CrawlOrderingSimulator(std::vector<SimPage> pages)
    : pages_(std::move(pages)) {
    for (const SimPage& page : pages_) {
        for (int link : page.links) {
            if (link < 0 || static_cast<size_t>(link) >= pages_.size())
                throw std::invalid_argument("Simulator graph contains an invalid page index");
        }
    }
}

CrawlRun CrawlOrderingSimulator::summarize(
    const std::string& name, const std::vector<int>& order,
    double importantThreshold) const {
    CrawlRun result;
    result.strategy = name;
    result.visitOrder = order;
    for (int id : order) {
        const double score = pages_[id].relevance;
        result.totalRelevance += score;
        if (score >= importantThreshold) ++result.importantFound;
    }
    if (!order.empty()) result.averageRelevance = result.totalRelevance / order.size();
    return result;
}

CrawlRun CrawlOrderingSimulator::runBfs(
    int start, int budget, double importantThreshold) const {
    if (start < 0 || static_cast<size_t>(start) >= pages_.size() || budget < 0)
        throw std::invalid_argument("Invalid BFS start or crawl budget");
    std::queue<int> frontier;
    std::vector<bool> seen(pages_.size(), false);
    std::vector<int> order;
    frontier.push(start);
    seen[start] = true;
    while (!frontier.empty() && static_cast<int>(order.size()) < budget) {
        int current = frontier.front();
        frontier.pop();
        order.push_back(current);
        for (int next : pages_[current].links) {
            if (!seen[next]) {
                seen[next] = true;
                frontier.push(next);
            }
        }
    }
    return summarize("BFS (first discovered)", order, importantThreshold);
}

CrawlRun CrawlOrderingSimulator::runFocused(
    int start, int budget, double importantThreshold) const {
    if (start < 0 || static_cast<size_t>(start) >= pages_.size() || budget < 0)
        throw std::invalid_argument("Invalid focused start or crawl budget");
    // Highest estimated topic relevance is selected first; ties use page index.
    struct Candidate { double score; int id; };
    auto lowerPriority = [](const Candidate& a, const Candidate& b) {
        if (a.score != b.score) return a.score < b.score;
        return a.id > b.id;
    };
    std::priority_queue<Candidate, std::vector<Candidate>, decltype(lowerPriority)> frontier(lowerPriority);
    std::vector<bool> discovered(pages_.size(), false);
    std::vector<bool> crawled(pages_.size(), false);
    std::vector<int> order;
    frontier.push({pages_[start].relevance, start});
    discovered[start] = true;
    while (!frontier.empty() && static_cast<int>(order.size()) < budget) {
        int current = frontier.top().id;
        frontier.pop();
        if (crawled[current]) continue;
        crawled[current] = true;
        order.push_back(current);
        for (int next : pages_[current].links) {
            if (!discovered[next]) {
                discovered[next] = true;
                frontier.push({pages_[next].relevance, next});
            }
        }
    }
    return summarize("Focused (relevance priority)", order, importantThreshold);
}

CrawlRun CrawlOrderingSimulator::runOpic(
    int start, int budget, double importantThreshold) const {
    if (start < 0 || static_cast<size_t>(start) >= pages_.size() || budget < 0)
        throw std::invalid_argument("Invalid OPIC start or crawl budget");
    // Simplified OPIC: a crawled page distributes its accumulated cash equally
    // among its outgoing links. The frontier page with the most cash is next.
    struct Candidate { double cash; int id; };
    auto lowerCash = [](const Candidate& a, const Candidate& b) {
        if (a.cash != b.cash) return a.cash < b.cash;
        return a.id > b.id;
    };
    std::priority_queue<Candidate, std::vector<Candidate>, decltype(lowerCash)> frontier(lowerCash);
    std::vector<double> cash(pages_.size(), 0.0);
    std::vector<bool> discovered(pages_.size(), false);
    std::vector<bool> crawled(pages_.size(), false);
    std::vector<int> order;
    cash[start] = 1.0;
    discovered[start] = true;
    frontier.push({cash[start], start});
    while (!frontier.empty() && static_cast<int>(order.size()) < budget) {
        Candidate candidate = frontier.top();
        frontier.pop();
        int current = candidate.id;
        if (crawled[current]) continue;
        crawled[current] = true;
        order.push_back(current);
        const auto& outgoing = pages_[current].links;
        if (!outgoing.empty()) {
            const double share = cash[current] / outgoing.size();
            for (int next : outgoing) {
                cash[next] += share;
                if (!discovered[next]) discovered[next] = true;
                if (!crawled[next]) frontier.push({cash[next], next});
            }
        }
    }
    return summarize("OPIC (cash priority)", order, importantThreshold);
}

std::vector<SimPage> makeDemoWebGraph() {
    // Offline synthetic site. Scores represent known topic relevance for evaluation;
    // the OPIC strategy itself does not use these scores when choosing pages.
    return {
        {"/",                 0.30, {1, 2, 3, 4}},
        {"/news/",            0.25, {5, 6}},
        {"/technology/",      0.95, {7, 8, 9}},
        {"/sports/",          0.10, {10}},
        {"/about/",           0.05, {11}},
        {"/news/local/",      0.20, {6}},
        {"/news/archive/",    0.15, {5}},
        {"/technology/ai/",   1.00, {8, 9}},
        {"/technology/cpp/",  0.98, {7, 9}},
        {"/technology/data/", 0.92, {7, 8}},
        {"/sports/football/", 0.08, {3}},
        {"/contact/",         0.02, {4}}
    };
}
