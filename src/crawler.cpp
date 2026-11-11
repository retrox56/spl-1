#include "crawler.h"

#include "link_extractor.h"
#include "url_utils.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <queue>
#include <thread>

using namespace std;

namespace {

struct Job {
    string url;
    int depth;
    string parent;
};

string lower(string s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c) { return tolower(c); });
    return s;
}

bool isHtml(const string& contentType) {
    return lower(contentType).find("html") != string::npos;
}

bool looksLikeFile(const string& url) {
    ParsedUrl p = parseUrl(url);
    size_t dot = p.path.rfind('.');
    size_t slash = p.path.rfind('/');
    if (dot == string::npos || dot < slash) return false;

    string ext = lower(p.path.substr(dot + 1));
    static const char* skip[] = {
        "jpg", "jpeg", "png", "gif", "svg", "webp", "ico", "pdf", "zip", "rar",
        "7z", "gz", "exe", "mp3", "mp4", "avi", "mov", "css", "js", "doc",
        "docx", "xls", "xlsx", "ppt", "pptx"
    };
    for (const char* s : skip) {
        if (ext == s) return true;
    }
    return false;
}

} // namespace

Crawler::Crawler(int maxPages, int maxDepth)
    : maxPages_(maxPages), maxDepth_(maxDepth), delayMs_(200),
      skippedFiles_(0), leftInQueue_(0) {
    client_.setTimeout(10);
}

void Crawler::run(const string& startUrl) {
    pages_.clear();
    visited_.clear();
    skippedFiles_ = 0;
    leftInQueue_ = 0;

    startUrl_ = normalizeUrl(startUrl);
    if (startUrl_.empty()) {
        cout << "Not a valid http/https URL: " << startUrl << "\n";
        return;
    }

    queue<Job> todo;
    todo.push({startUrl_, 0, ""});
    visited_.insert(startUrl_);

    while (!todo.empty() && (int)pages_.size() < maxPages_) {
        Job job = todo.front();
        todo.pop();

        if (!pages_.empty() && delayMs_ > 0) {
            this_thread::sleep_for(chrono::milliseconds(delayMs_));
        }

        PageInfo page = visit(job.url, job.depth, job.parent);
        pages_.push_back(page);
        printProgress(page, pages_.size());

        for (const string& link : page.links) {
            if (!isSameHost(link, startUrl_)) continue;
            if (!visited_.insert(link).second) continue;
            if (looksLikeFile(link)) {
                skippedFiles_++;
                continue;
            }
            if (job.depth + 1 > maxDepth_) continue;
            todo.push({link, job.depth + 1, page.finalUrl});
        }
    }
    leftInQueue_ = (int)todo.size();
}

PageInfo Crawler::visit(const string& url, int depth, const string& parent) {
    PageInfo page;
    page.url = url;
    page.finalUrl = url;
    page.depth = depth;
    page.parent = parent;

    const int maxRedirects = 5;
    string current = url;
    string html;

    for (int hop = 0;; hop++) {
        HttpResponse r = client_.get(current);
        page.seconds += r.totalTimeSec;

        if (!r.success) {
            page.error = r.errorMessage;
            break;
        }

        page.reached = true;
        page.statusCode = r.statusCode;
        page.contentType = r.contentType;

        bool isRedirect = r.statusCode >= 300 && r.statusCode < 400 && !r.location.empty();
        if (!isRedirect) {
            html = r.body;
            break;
        }
        if (hop == maxRedirects) {
            page.error = "too many redirects";
            break;
        }

        string next = resolveUrl(current, r.location);
        if (next.empty()) {
            page.error = "bad redirect target";
            break;
        }
        page.redirects.push_back(next);
        current = next;
    }
    page.finalUrl = current;

    if (depth == 0 && !page.redirects.empty()) startUrl_ = current;

    bool alreadySeen = false;
    if (current != url) alreadySeen = !visited_.insert(current).second;

    if (alreadySeen || !page.error.empty() || page.statusCode != 200) return page;
    if (!isHtml(page.contentType)) return page;
    if (!isSameHost(current, startUrl_)) return page;

    unordered_set<string> seen;
    for (const string& raw : extractLinks(html)) {
        string full = resolveUrl(current, raw);
        if (full.empty() || full == current) continue;
        if (seen.insert(full).second) page.links.push_back(full);
    }
    return page;
}

void Crawler::printProgress(const PageInfo& page, size_t number) const {
    cout << "[" << setw(3) << number << "] d" << page.depth << "  ";

    if (!page.reached) {
        cout << "ERR  " << page.url << "  (" << page.error << ")\n";
        return;
    }

    cout << page.statusCode << "  " << page.url;
    if (!page.redirects.empty()) cout << "  -> " << page.finalUrl;
    if (!page.links.empty()) cout << "  [" << page.links.size() << " links]";
    if (!page.error.empty()) cout << "  (" << page.error << ")";
    cout << "\n";
}