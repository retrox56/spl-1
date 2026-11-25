#include "crawl_ordering_simulator.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

static void printRun(const CrawlRun& run, const std::vector<SimPage>& pages) {
    std::cout << "\n" << run.strategy << "\n";
    std::cout << "Visit order: ";
    for (size_t i = 0; i < run.visitOrder.size(); ++i) {
        if (i) std::cout << " -> ";
        std::cout << pages[run.visitOrder[i]].url;
    }
    std::cout << "\nPages crawled: " << run.visitOrder.size()
              << "\nImportant pages found: " << run.importantFound
              << "\nTotal relevance: " << std::fixed << std::setprecision(2)
              << run.totalRelevance
              << "\nAverage relevance: " << run.averageRelevance << "\n";
}

int main(int argc, char* argv[]) {
    int budget = 8;
    if (argc >= 2) {
        try { budget = std::stoi(argv[1]); }
        catch (...) {
            std::cerr << "Usage: crawl_ordering_lab.exe [crawl-budget]\n";
            return 1;
        }
    }
    if (budget < 1) {
        std::cerr << "Crawl budget must be at least 1.\n";
        return 1;
    }

    const double importantThreshold = 0.70;
    CrawlOrderingSimulator simulator(makeDemoWebGraph());
    const auto& pages = simulator.pages();

    std::cout << "==============================================\n";
    std::cout << " OFFLINE CRAWL-ORDERING RESEARCH LAB\n";
    std::cout << "==============================================\n";
    std::cout << "Synthetic website pages: " << pages.size()
              << "\nCrawl budget: " << budget
              << "\nImportant-page threshold: " << importantThreshold
              << "\nNo internet connection is used.\n";

    std::cout << "\nSIMULATED WEBSITE (URL : relevance score)\n";
    for (size_t i = 0; i < pages.size(); ++i)
        std::cout << std::setw(2) << i << "  " << std::left << std::setw(23)
                  << pages[i].url << " relevance=" << pages[i].relevance << "\n";

    CrawlRun bfs = simulator.runBfs(0, budget, importantThreshold);
    CrawlRun focused = simulator.runFocused(0, budget, importantThreshold);
    CrawlRun opic = simulator.runOpic(0, budget, importantThreshold);
    printRun(bfs, pages);
    printRun(focused, pages);
    printRun(opic, pages);

    std::cout << "\n==============================================\n";
    std::cout << "COMPARISON (higher is better)\n";
    std::cout << std::left << std::setw(30) << "Strategy"
              << std::setw(18) << "Important found" << "Avg relevance\n";
    for (const CrawlRun* run : {&bfs, &focused, &opic})
        std::cout << std::left << std::setw(30) << run->strategy
                  << std::setw(18) << run->importantFound
                  << std::fixed << std::setprecision(2) << run->averageRelevance << "\n";
    std::cout << "\nNote: relevance scores are simulator ground truth. Focused crawling\n"
              << "uses them as its estimate; OPIC uses only link-flow cash.\n";
    return 0;
}
