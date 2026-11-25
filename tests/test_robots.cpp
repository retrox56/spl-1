#include "robots_parser.h"
#include <cassert>
#include <iostream>

int main() {
    RobotsParser r;
    r.parse("User-agent: *\nDisallow: /private\nAllow: /private/public\nDisallow: /*.pdf$\n", "SPL1-WebCrawler");
    assert(r.isAllowed("https://example.com/"));
    assert(!r.isAllowed("https://example.com/private/account"));
    assert(r.isAllowed("https://example.com/private/public/info"));
    assert(!r.isAllowed("https://example.com/files/a.pdf"));
    assert(r.isAllowed("https://example.com/files/a.pdf?download=1"));

    r.parse("User-agent: OtherBot\nDisallow: /\nUser-agent: *\nDisallow: /hidden\n", "SPL1-WebCrawler");
    assert(r.isAllowed("https://example.com/"));
    assert(!r.isAllowed("https://example.com/hidden/page"));

    r.parse("User-agent: *\nDisallow: /same\nAllow: /same\n", "SPL1-WebCrawler");
    assert(r.isAllowed("https://example.com/same"));
    std::cout << "OK robots parser tests\n";
}
