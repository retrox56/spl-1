#ifndef LINK_EXTRACTOR_H
#define LINK_EXTRACTOR_H

#include <string>
#include <vector>

std::vector<std::string> extractLinks(const std::string& html);
std::string decodeHtmlEntities(const std::string& text);

#endif