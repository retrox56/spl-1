#include "url_utils.h"

#include <algorithm>
#include <cctype>
#include <vector>

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

int defaultPort(const std::string& scheme) {
    return scheme == "https" ? 443 : 80;
}

// If 'link' starts with a scheme like "mailto:" or "https:", return it (lowercase).
// Returns "" if the link has no scheme (it is relative).
std::string getScheme(const std::string& link) {
    for (size_t i = 0; i < link.size(); i++) {
        char c = link[i];
        if (c == ':') {
            if (i == 0) return "";
            return toLower(link.substr(0, i));
        }
        if (c == '/' || c == '?' || c == '#') return "";
        bool ok = std::isalnum(static_cast<unsigned char>(c)) || c == '+' || c == '-' || c == '.';
        if (!ok) return "";
    }
    return "";
}


void splitPathQuery(const std::string& s, std::string& path, std::string& query) {
    size_t q = s.find('?');
    if (q == std::string::npos) {
        path = s;
        query.clear();
    } else {
        path = s.substr(0, q);
        query = s.substr(q + 1);
    }
}

} 

ParsedUrl parseUrl(const std::string& rawUrl) {
    ParsedUrl result;
    std::string url = trim(rawUrl);

    // 1. scheme
    size_t schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos) return result;
    result.scheme = toLower(url.substr(0, schemeEnd));
    if (result.scheme != "http" && result.scheme != "https") return result;

    std::string rest = url.substr(schemeEnd + 3);

    // 2. drop the #fragment
    size_t hash = rest.find('#');
    if (hash != std::string::npos) rest.erase(hash);

    // 3. split authority (host[:port]) from the rest
    size_t authEnd = rest.find_first_of("/?");
    std::string authority = (authEnd == std::string::npos) ? rest : rest.substr(0, authEnd);
    std::string remainder = (authEnd == std::string::npos) ? "" : rest.substr(authEnd);

    // remove "user:pass@" if present
    size_t at = authority.find('@');
    if (at != std::string::npos) authority = authority.substr(at + 1);

    if (!authority.empty() && authority[0] == '[') return result;  // IPv6 not supported yet

    // 4. host and port
    std::string host = authority;
    size_t colon = authority.find(':');
    if (colon != std::string::npos) {
        host = authority.substr(0, colon);
        std::string portText = authority.substr(colon + 1);
        if (!portText.empty()) {
            if (portText.size() > 5) return result;
            int port = 0;
            for (char c : portText) {
                if (!std::isdigit(static_cast<unsigned char>(c))) return result;
                port = port * 10 + (c - '0');
            }
            result.port = (port == defaultPort(result.scheme)) ? 0 : port;
        }
    }
    if (host.empty()) return result;
    result.host = toLower(host);

    // 5. path and query
    splitPathQuery(remainder, result.path, result.query);
    if (result.path.empty()) result.path = "/";

    result.valid = true;
    return result;
}

std::string removeDotSegments(const std::string& rawPath) {
    std::string path = rawPath;
    if (path.empty() || path[0] != '/') path = "/" + path;

    // Split on '/'. "/a/b" 
    std::vector<std::string> parts;
    size_t start = 0;
    while (true) {
        size_t slash = path.find('/', start);
        if (slash == std::string::npos) {
            parts.push_back(path.substr(start));
            break;
        }
        parts.push_back(path.substr(start, slash - start));
        start = slash + 1;
    }

    std::vector<std::string> stack;
    bool trailingSlash = false;
    for (size_t k = 1; k < parts.size(); k++) {
        const std::string& seg = parts[k];
        bool isLast = (k == parts.size() - 1);

        if (seg == ".") {
            if (isLast) trailingSlash = true;
        } else if (seg == "..") {
            if (!stack.empty()) stack.pop_back();
            if (isLast) trailingSlash = true;
        } else if (seg.empty()) {
            if (isLast) trailingSlash = true;   // "/a/" ends with a slash
        } else {
            stack.push_back(seg);
        }
    }

    std::string result = "/";
    for (size_t i = 0; i < stack.size(); i++) {
        if (i > 0) result += "/";
        result += stack[i];
    }
    if (trailingSlash && !stack.empty()) result += "/";
    return result;
}

std::string buildUrl(const ParsedUrl& parts) {
    std::string url = parts.scheme + "://" + parts.host;
    if (parts.port != 0) url += ":" + std::to_string(parts.port);
    url += parts.path;
    if (!parts.query.empty()) url += "?" + parts.query;
    return url;
}

std::string normalizeUrl(const std::string& url) {
    ParsedUrl parts = parseUrl(url);
    if (!parts.valid) return "";

    parts.path = removeDotSegments(parts.path);

    // spaces are not allowed in URLs; encode them
    std::string encoded;
    for (char c : parts.path) {
        if (c == ' ') encoded += "%20";
        else encoded += c;
    }
    parts.path = encoded;

    return buildUrl(parts);
}

std::string resolveUrl(const std::string& baseUrl, const std::string& rawLink) {
    std::string link = trim(rawLink);
    if (link.empty()) return "";
    if (link[0] == '#') return "";   // link to a spot on the same page

    // Link has its own scheme: accept http/https, reject mailto:, javascript:, tel:, ...
    std::string scheme = getScheme(link);
    if (!scheme.empty()) {
        if (scheme == "http" || scheme == "https") return normalizeUrl(link);
        return "";
    }

    ParsedUrl base = parseUrl(baseUrl);
    if (!base.valid) return "";

    // drop the #fragment from the link
    size_t hash = link.find('#');
    if (hash != std::string::npos) link.erase(hash);
    if (link.empty()) return "";

    // "//host/path" keeps the base scheme
    if (link.compare(0, 2, "//") == 0) {
        return normalizeUrl(base.scheme + ":" + link);
    }

    ParsedUrl result = base;

    if (link[0] == '?') {
        // "?page=2": same path, new query
        result.query = link.substr(1);
    } else if (link[0] == '/') {
        // "/about": absolute path on the same host
        splitPathQuery(link, result.path, result.query);
    } else {
        // "contact.html" or "../up.html": relative to the base's directory
        std::string linkPath, linkQuery;
        splitPathQuery(link, linkPath, linkQuery);
        size_t lastSlash = base.path.rfind('/');
        std::string baseDir = base.path.substr(0, lastSlash + 1);
        result.path = baseDir + linkPath;
        result.query = linkQuery;
    }

    result.path = removeDotSegments(result.path);
    return normalizeUrl(buildUrl(result));
}

bool isSameHost(const std::string& urlA, const std::string& urlB) {
    ParsedUrl a = parseUrl(urlA);
    ParsedUrl b = parseUrl(urlB);
    return a.valid && b.valid && a.host == b.host;
}