#include "link_extractor.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int failed = 0;

void test(const string& name, const string& html, vector<string> expected) {
    vector<string> got = extractLinks(html);
    if (got == expected) {
        cout << "ok    " << name << "\n";
        return;
    }
    failed++;
    cout << "FAIL  " << name << "\n  wanted:";
    for (const string& s : expected) cout << " [" << s << "]";
    cout << "\n  got:   ";
    for (const string& s : got) cout << " [" << s << "]";
    cout << "\n";
}

int main() {
    test("double quotes", R"(<a href="/a">x</a>)", {"/a"});
    test("single quotes", R"(<a href='b.html'>x</a>)", {"b.html"});
    test("no quotes", R"(<a href=c.html>x</a>)", {"c.html"});
    test("uppercase", R"(<A HREF="D.html">x</A>)", {"D.html"});
    test("href not first", R"(<a class="btn" id=x href="e.html" target=_blank>x</a>)", {"e.html"});
    test("&amp; decoded", R"(<a href="/s?a=1&amp;b=2">x</a>)", {"/s?a=1&b=2"});
    test("comment ignored", R"(<!-- <a href="hidden"> --><a href="shown">x</a>)", {"shown"});
    test("script ignored", R"(<script>var s="<a href='x'>";</script><a href="y">z</a>)", {"y"});
    test("style ignored", R"(<style>a{}</style><a href="s">z</a>)", {"s"});
    test("anchor without href", R"(<a name="top">x</a>)", {});
    test("abbr and area ignored", R"(<abbr title="x">a</abbr><area href="no">)", {});
    test("link tag ignored", R"(<link href="style.css"><a href="yes">x</a>)", {"yes"});
    test("> inside an attribute", R"(<a title="a>b" href="/q">x</a>)", {"/q"});
    test("newline inside tag", "<a\nhref=\"/n\">x</a>", {"/n"});
    test("empty page", "", {});
    test("three links in order", R"(<a href="/1">1</a> text <a href="/2">2</a><a href="/3">3</a>)",
         {"/1", "/2", "/3"});

    cout << "\n" << (failed == 0 ? "all passed" : "some failed") << " (" << failed << " failures)\n";
    return failed == 0 ? 0 : 1;
}