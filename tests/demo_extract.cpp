#include "http_client.h"
#include "link_extractor.h"
#include "url_utils.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

void printGroup(const string& title, const vector<string>& items) {
    cout << "\n" << title << " (" << items.size() << ")\n";
    for (const string& item : items) cout << "  " << item << "\n";
}

void showLinks(const string& pageUrl, const string& html) {
    vector<string> raw = extractLinks(html);
    unordered_set<string> seen;
    vector<string> internal, external, skipped, duplicates;

    for (const string& link : raw) {
        string full = resolveUrl(pageUrl, link);
        string line = link + "  ->  " + full;

        if (full.empty()) skipped.push_back(link);
        else if (!seen.insert(full).second) duplicates.push_back(line);
        else if (isSameHost(pageUrl, full)) internal.push_back(line);
        else external.push_back(line);
    }

    cout << "\n" << pageUrl << "\n" << raw.size() << " links found in the HTML\n";
    printGroup("Internal pages", internal);
    printGroup("External sites", external);
    printGroup("Duplicates (already seen)", duplicates);
    printGroup("Skipped (not crawlable)", skipped);
}

int readFile(const string& path, const string& baseUrl) {
    ifstream in(path, ios::binary);
    if (!in) {
        cout << "Could not open " << path << "\n";
        return 1;
    }
    stringstream text;
    text << in.rdbuf();
    showLinks(baseUrl, text.str());
    return 0;
}

int fetchSite(const string& startUrl) {
    HttpClient client;
    string url = startUrl;

    for (int hops = 0; hops < 8; hops++) {
        HttpResponse r = client.get(url);
        if (!r.success) {
            cout << "Could not reach " << url << ": " << r.errorMessage << "\n";
            return 1;
        }
        cout << r.statusCode << "  " << url << "\n";

        bool redirect = r.statusCode >= 300 && r.statusCode < 400 && !r.location.empty();
        if (redirect) {
            string next = resolveUrl(url, r.location);
            if (next.empty()) {
                cout << "Bad redirect target: " << r.location << "\n";
                return 1;
            }
            cout << "     moved to " << next << "\n";
            url = next;
            continue;
        }

        if (r.statusCode == 200 && r.contentType.find("html") != string::npos) {
            showLinks(url, r.body);
        } else {
            cout << "Nothing to read here (type: " << r.contentType << ")\n";
        }
        return 0;
    }
    cout << "Gave up after too many redirects\n";
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc == 4 && string(argv[1]) == "file") return readFile(argv[2], argv[3]);
    if (argc == 3 && string(argv[1]) == "url") return fetchSite(argv[2]);

    cout << "demo_extract file <html file> <pretend page url>\n"
         << "demo_extract url <website>\n";
    return 1;
}