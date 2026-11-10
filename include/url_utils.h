#ifndef URL_UTILS_H
#define URL_UTILS_H

#include <string>

// A URL split into its parts.
struct ParsedUrl {
    bool valid = false;
    std::string scheme;   // "http" or "https" (lowercase)
    std::string host;     // lowercase, e.g. "example.com"
    int port = 0;         // 0 means "default port for the scheme"
    std::string path;     // always starts with '/'
    std::string query;    // text after '?', without the '?'
};

// Split an absolute http/https URL. Returns valid=false if it can't be parsed.
ParsedUrl parseUrl(const std::string& url);

// Turn "/a/./b/../c" into "/a/c".
std::string removeDotSegments(const std::string& path);

// Put a ParsedUrl back together as a string.
std::string buildUrl(const ParsedUrl& parts);

// Canonical form of an absolute URL. Returns "" if the URL is not valid http/https.
std::string normalizeUrl(const std::string& url);

// Turn a link found on a page into a full normalized URL.
// Returns "" for links that should not be crawled (mailto:, javascript:, #fragment-only, empty...).
std::string resolveUrl(const std::string& baseUrl, const std::string& link);

// True if both URLs have the same host.
bool isSameHost(const std::string& urlA, const std::string& urlB);

#endif