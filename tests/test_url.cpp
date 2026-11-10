#include "url_utils.h"
#include <iostream>
#include <string>

static int failures = 0;

void check(const std::string& label, const std::string& got, const std::string& expected) {
    bool ok = (got == expected);
    if (!ok) failures++;
    std::cout << (ok ? "PASS  " : "FAIL  ") << label << "\n";
    if (!ok) {
        std::cout << "      expected: [" << expected << "]\n"
                  << "      got:      [" << got << "]\n";
    }
}

int main() {
    const std::string base = "http://Example.com/a/b/page.html?x=1#top";

    // normalizeUrl
    check("normalize mixed case, port 80, dots, fragment",
          normalizeUrl("HTTP://Example.COM:80/a/./b/../c#frag"), "http://example.com/a/c");
    check("normalize empty path", normalizeUrl("https://example.com"), "https://example.com/");
    check("normalize default https port", normalizeUrl("https://example.com:443/x"), "https://example.com/x");
    check("normalize keeps custom port", normalizeUrl("https://example.com:8080/x"), "https://example.com:8080/x");
    check("normalize keeps query", normalizeUrl("http://example.com/s?q=1&p=2"), "http://example.com/s?q=1&p=2");
    check("normalize keeps trailing slash", normalizeUrl("http://example.com/dir/"), "http://example.com/dir/");
    check("normalize rejects ftp", normalizeUrl("ftp://example.com/file"), "");
    check("normalize rejects garbage", normalizeUrl("not a url"), "");

    // resolveUrl
    check("resolve absolute path", resolveUrl(base, "/about"), "http://example.com/about");
    check("resolve relative file", resolveUrl(base, "contact.html"), "http://example.com/a/b/contact.html");
    check("resolve ../", resolveUrl(base, "../up.html"), "http://example.com/a/up.html");
    check("resolve too many ../", resolveUrl(base, "../../../x"), "http://example.com/x");
    check("resolve protocol-relative", resolveUrl(base, "//cdn.example.com/lib.js"), "http://cdn.example.com/lib.js");
    check("resolve absolute other site", resolveUrl(base, "https://other.org/p"), "https://other.org/p");
    check("resolve query only", resolveUrl(base, "?page=2"), "http://example.com/a/b/page.html?page=2");
    check("resolve fragment only", resolveUrl(base, "#top"), "");
    check("resolve mailto", resolveUrl(base, "mailto:a@b.com"), "");
    check("resolve javascript", resolveUrl(base, "javascript:void(0)"), "");
    check("resolve tel", resolveUrl(base, "tel:+8801700000000"), "");
    check("resolve empty", resolveUrl(base, ""), "");
    check("resolve with fragment", resolveUrl(base, "/x.html#sec"), "http://example.com/x.html");
    check("resolve against directory base", resolveUrl("https://example.com/dir/", "x"), "https://example.com/dir/x");
    check("resolve against bare host", resolveUrl("https://example.com", "x"), "https://example.com/x");

    // isSameHost
    check("same host", isSameHost("http://a.com/x", "https://A.com/y") ? "yes" : "no", "yes");
    check("different host", isSameHost("http://a.com/x", "http://b.com/x") ? "yes" : "no", "no");

    std::cout << "\n" << (failures == 0 ? "All tests passed." : "Some tests FAILED.")
              << " Failures: " << failures << "\n";
    return failures == 0 ? 0 : 1;
}