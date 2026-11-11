#include "crawler.h"
#include "url_utils.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

bool isBroken(const PageInfo& p) {
    return !p.reached || !p.error.empty() || p.statusCode >= 400;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "usage: crawler <url> [maxPages] [maxDepth] [delayMs]\n";
        return 1;
    }

    string startUrl = argv[1];
    int maxPages = argc > 2 ? atoi(argv[2]) : 30;
    int maxDepth = argc > 3 ? atoi(argv[3]) : 2;
    int delayMs = argc > 4 ? atoi(argv[4]) : 200;
    string caBundlePath = argc > 5 ? argv[5] : "";

    if (maxPages < 1 || maxDepth < 0 || delayMs < 0) {
        cout << "maxPages must be at least 1, maxDepth and delayMs cannot be negative\n";
        return 1;
    }

    Crawler crawler(maxPages, maxDepth);
    crawler.setDelayMs(delayMs);

    if (!caBundlePath.empty()) {
    crawler.setCaBundlePath(caBundlePath);
    }

    cout << "Crawling " << startUrl << "  (max " << maxPages << " pages, depth "
         << maxDepth << ", " << delayMs << " ms delay)\n\n";

    auto begin = chrono::steady_clock::now();
    crawler.run(startUrl);
    double total = chrono::duration<double>(chrono::steady_clock::now() - begin).count();

    const vector<PageInfo>& pages = crawler.pages();
    if (pages.empty()) return 1;

    int ok = 0, broken = 0, redirected = 0;
    double requestTime = 0;
    vector<int> perDepth(maxDepth + 1, 0);
    unordered_set<string> allLinks, externalLinks;

    for (const PageInfo& p : pages) {
        if (isBroken(p)) broken++;
        else if (p.statusCode == 200) ok++;
        if (!p.redirects.empty()) redirected++;
        requestTime += p.seconds;
        perDepth[p.depth]++;

        for (const string& link : p.links) {
            allLinks.insert(link);
            if (!isSameHost(link, pages[0].finalUrl)) externalLinks.insert(link);
        }
    }

    cout << "\n===== Summary =====\n";
    cout << "pages crawled        : " << pages.size() << "\n";
    cout << "  ok (200)           : " << ok << "\n";
    cout << "  redirected         : " << redirected << "\n";
    cout << "  broken / failed    : " << broken << "\n";
    cout << "still in queue       : " << crawler.leftInQueue() << "\n";
    cout << "files skipped        : " << crawler.skippedFiles() << "\n";
    cout << "unique links found   : " << allLinks.size() << "\n";
    cout << "  external           : " << externalLinks.size() << "\n";
    cout << "avg response time    : " << fixed << setprecision(3)
         << requestTime / pages.size() << " s\n";
    cout << "total time           : " << setprecision(1) << total << " s\n";

    cout << "\nPages per depth\n";
    for (int d = 0; d <= maxDepth; d++) {
        cout << "  depth " << d << " : " << perDepth[d] << "\n";
    }

    vector<const PageInfo*> byLinks;
    for (const PageInfo& p : pages) byLinks.push_back(&p);
    sort(byLinks.begin(), byLinks.end(),
         [](const PageInfo* a, const PageInfo* b) { return a->links.size() > b->links.size(); });

    cout << "\nPages with the most outgoing links\n";
    for (size_t i = 0; i < byLinks.size() && i < 5; i++) {
        cout << "  " << byLinks[i]->links.size() << "  " << byLinks[i]->finalUrl << "\n";
    }

    if (redirected > 0) {
        cout << "\nRedirect chains\n";
        for (const PageInfo& p : pages) {
            if (p.redirects.empty()) continue;
            cout << "  " << p.url;
            for (const string& hop : p.redirects) cout << " -> " << hop;
            cout << "  (" << p.statusCode << ")\n";
        }
    }

    if (broken > 0) {
        cout << "\nBroken links\n";
        for (const PageInfo& p : pages) {
            if (!isBroken(p)) continue;
            cout << "  " << p.url << "\n    problem  : ";
            if (!p.error.empty()) cout << p.error << "\n";
            else cout << "HTTP " << p.statusCode << "\n";
            cout << "    found on : " << (p.parent.empty() ? "(start page)" : p.parent) << "\n";
        }
    }
    return 0;
}