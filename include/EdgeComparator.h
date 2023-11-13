#pragma once
#include "Edge.h"

struct EdgeComparator {
    bool operator ()(const Edge* a, const Edge* b) {
        return a->weight > b->weight;
    }
};
