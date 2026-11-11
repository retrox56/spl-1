#include "link_extractor.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <utility>

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

bool isSpace(char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

// "<a href" matches tag "a", "<abbr" does not
bool startsWithTag(const std::string& lower, size_t pos, const std::string& name) {
    if (lower.compare(pos + 1, name.size(), name) != 0) return false;
    size_t after = pos + 1 + name.size();
    if (after >= lower.size()) return false;
    char c = lower[after];
    return isSpace(c) || c == '>' || c == '/';
}

// a '>' inside quotes does not end the tag
size_t findTagEnd(const std::string& html, size_t start) {
    char quote = 0;
    for (size_t k = start + 1; k < html.size(); k++) {
        char c = html[k];
        if (quote) {
            if (c == quote) quote = 0;
        } else if (c == '"' || c == '\'') {
            quote = c;
        } else if (c == '>') {
            return k;
        }
    }
    return std::string::npos;
}

std::string getAttribute(const std::string& tag, const std::string& name) {
    size_t n = tag.size();
    size_t k = 2;
    while (k < n) {
        while (k < n && (isSpace(tag[k]) || tag[k] == '/')) k++;
        if (k >= n || tag[k] == '>') break;

        size_t nameStart = k;
        while (k < n && !isSpace(tag[k]) && tag[k] != '=' && tag[k] != '>' && tag[k] != '/') k++;
        std::string attrName = toLower(tag.substr(nameStart, k - nameStart));

        while (k < n && isSpace(tag[k])) k++;

        std::string value;
        if (k < n && tag[k] == '=') {
            k++;
            while (k < n && isSpace(tag[k])) k++;
            if (k < n && (tag[k] == '"' || tag[k] == '\'')) {
                char quote = tag[k++];
                size_t valueStart = k;
                while (k < n && tag[k] != quote) k++;
                value = tag.substr(valueStart, k - valueStart);
                if (k < n) k++;
            } else {
                size_t valueStart = k;
                while (k < n && !isSpace(tag[k]) && tag[k] != '>') k++;
                value = tag.substr(valueStart, k - valueStart);
            }
        }
        if (attrName == name) return value;
    }
    return "";
}

} // namespace

std::string decodeHtmlEntities(const std::string& text) {
    static const std::pair<const char*, char> entities[] = {
        {"&amp;", '&'}, {"&lt;", '<'}, {"&gt;", '>'}, {"&quot;", '"'},
        {"&#39;", '\''}, {"&apos;", '\''}, {"&#x27;", '\''}, {"&#38;", '&'}
    };

    std::string out;
    size_t i = 0;
    while (i < text.size()) {
        bool matched = false;
        if (text[i] == '&') {
            for (const auto& e : entities) {
                size_t len = std::strlen(e.first);
                if (text.compare(i, len, e.first) == 0) {
                    out += e.second;
                    i += len;
                    matched = true;
                    break;
                }
            }
        }
        if (!matched) out += text[i++];
    }
    return out;
}

std::vector<std::string> extractLinks(const std::string& html) {
    std::vector<std::string> links;
    std::string lower = toLower(html);
    size_t i = 0;

    while (i < html.size()) {
        size_t lt = lower.find('<', i);
        if (lt == std::string::npos) break;

        if (lower.compare(lt, 4, "<!--") == 0) {
            size_t end = lower.find("-->", lt + 4);
            if (end == std::string::npos) break;
            i = end + 3;
            continue;
        }

        bool isScript = startsWithTag(lower, lt, "script");
        if (isScript || startsWithTag(lower, lt, "style")) {
            size_t close = lower.find(isScript ? "</script" : "</style", lt);
            if (close == std::string::npos) break;
            i = close;
            continue;
        }

        if (startsWithTag(lower, lt, "a")) {
            size_t tagEnd = findTagEnd(html, lt);
            if (tagEnd == std::string::npos) break;
            std::string href = getAttribute(html.substr(lt, tagEnd - lt + 1), "href");
            if (!href.empty()) links.push_back(decodeHtmlEntities(href));
            i = tagEnd + 1;
            continue;
        }

        i = lt + 1;
    }
    return links;
}