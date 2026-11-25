#ifndef ROBOTS_PARSER_H
#define ROBOTS_PARSER_H

#include <string>
#include <vector>

struct RobotsRule {
    std::string pattern;
    bool allow = false;
};

class RobotsParser {
public:
    void parse(const std::string& text, const std::string& userAgent);
    bool isAllowed(const std::string& url) const;
    bool isLoaded() const { return loaded_; }
    void setLoaded(bool value) { loaded_ = value; }
    size_t ruleCount() const { return rules_.size(); }

private:
    std::vector<RobotsRule> rules_;
    bool loaded_ = false;
};

#endif
