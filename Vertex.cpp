#include "include/Vertex.h"

Vertex::Vertex(std::string vertexName, std::vector<std::string> neighbors) {
    this->vertexName = vertexName;
    this->neighbors = neighbors;
}

void Vertex::setPosition(sf::Vector2f pos) {
    this->vertexRect.setPosition(pos);
}

void Vertex::setDimensions(sf::Vector2f dimensions) {
    this->vertexRect.setSize(dimensions);
}

void Vertex::setColor(sf::Color color) {
    this->vertexRect.setFillColor(color);
}

