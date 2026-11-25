#include "robots_parser.h"
#include "url_utils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace {
std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}
std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Glob matching for robots patterns: '*' matches any sequence; '$' anchors the end.
bool globMatch(const std::string& value, const std::string& pattern, size_t i = 0, size_t j = 0) {
    std::vector<std::vector<signed char>> memo(value.size() + 1, std::vector<signed char>(pattern.size() + 1, -1));
    auto solve = [&](auto&& self, size_t x, size_t y) -> bool {
        signed char& m = memo[x][y];
        if (m != -1) return m != 0;
        bool result;
        if (y == pattern.size() || pattern[y] == '$') result = (y == pattern.size() || x == value.size());
        else if (pattern[y] == '*') result = self(self, x, y + 1) || (x < value.size() && self(self, x + 1, y));
        else result = x < value.size() && value[x] == pattern[y] && self(self, x + 1, y + 1);
        m = result ? 1 : 0;
        return result;
    };
    (void)i; (void)j;
    return solve(solve, 0, 0);
}
size_t specificity(const std::string& p) {
    size_t n = 0;
    for (char c : p) if (c != '*' && c != '$') ++n;
    return n;
}
}

void RobotsParser::parse(const std::string& text, const std::string& userAgent) {
    rules_.clear();
    loaded_ = true;
    struct Group { std::vector<std::string> agents; std::vector<RobotsRule> rules; };
    std::vector<Group> groups;
    Group current;
    bool hasRules = false;
    std::istringstream input(text);
    std::string line;
    while (std::getline(input, line)) {
        size_t comment = line.find('#');
        if (comment != std::string::npos) line.erase(comment);
        line = trim(line);
        if (line.empty()) continue;
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string key = lower(trim(line.substr(0, colon)));
        std::string value = trim(line.substr(colon + 1));
        if (key == "user-agent") {
            if (hasRules) { groups.push_back(current); current = Group(); hasRules = false; }
            current.agents.push_back(lower(value));
        } else if ((key == "allow" || key == "disallow") && !current.agents.empty()) {
            hasRules = true;
            if (!value.empty()) current.rules.push_back({value, key == "allow"});
        }
    }
    if (!current.agents.empty()) groups.push_back(current);

    std::string ua = lower(userAgent);
    size_t bestAgentLength = 0;
    for (const Group& g : groups) {
        size_t groupMatch = 0;
        for (const std::string& agent : g.agents) {
            if (agent == "*") groupMatch = std::max(groupMatch, size_t(1));
            else if (!agent.empty() && ua.find(agent) != std::string::npos)
                groupMatch = std::max(groupMatch, agent.size());
        }
        if (groupMatch > bestAgentLength) {
            bestAgentLength = groupMatch;
            rules_ = g.rules;
        } else if (groupMatch != 0 && groupMatch == bestAgentLength) {
            rules_.insert(rules_.end(), g.rules.begin(), g.rules.end());
        }
    }
}

bool RobotsParser::isAllowed(const std::string& url) const {
    if (!loaded_) return true;
    ParsedUrl parsed = parseUrl(url);
    if (!parsed.valid) return false;
    std::string target = parsed.path.empty() ? "/" : parsed.path;
    if (!parsed.query.empty()) target += "?" + parsed.query;
    size_t best = 0;
    bool matched = true;
    for (const RobotsRule& rule : rules_) {
        if (!globMatch(target, rule.pattern)) continue;
        size_t score = specificity(rule.pattern);
        if (score > best || (score == best && rule.allow)) {
            best = score;
            matched = rule.allow;
        }
    }
    return matched;
}
