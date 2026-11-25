#ifndef PAGERANK_H
#define PAGERANK_H

#include "website_graph.h"
#include <vector>

std::vector<double> calculatePageRank(const WebsiteGraph& graph,
                                      double damping = 0.85,
                                      int maxIterations = 100,
                                      double tolerance = 1e-10);

#endif
