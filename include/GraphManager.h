#pragma once

#include <unordered_map>
#include <vector>
#include <algorithm>

#include "Vertex.h"
#include "Edge.h"

class GraphManager {
public:
    inline GraphManager() {}
    ~GraphManager();

    void addVertex(std::string vertexName, std::vector<std::string> neighbors, float posX, float PosY);
    void addEdge(std::string src, std::string dst, float weight);
    void clearVertices();
    void clearEdges();
    void renderText(std::string label, sf::Vector2f pos, sf::Font& font, int size, sf::Color color, sf::RenderWindow& window);
    void sortEdges();

    std::unordered_map<std::string, Vertex* >* getVerticesMap();
    std::vector<Edge* >* getEdges();

    Edge* getOneEdge(std::string src, std::string dst);

private:
    std::vector<Edge* > edges;
    std::unordered_map<std::string, Vertex* > vertices;
};

