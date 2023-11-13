#pragma once
#include <string>

struct Edge {
    std::string src;
    std::string dst;
    float weight;

    inline Edge(std::string src, std::string dst, float weight) {
        this->src = src;
        this->dst = dst;
        this->weight = weight;
    }
};

