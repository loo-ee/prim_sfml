#include <SFML/Graphics.hpp>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <queue>

#include "include/GraphManager.h"
#include "include/Button.h"
#include "include/EdgeComparator.h"

#define WHITE sf::Color::White
#define BLACK sf::Color::Black
#define GREEN sf::Color::Green
#define BLUE sf::Color::Blue
#define RED sf::Color::Red

void addVertexFromUser(GraphManager& manager, std::string& inputText);
void addVerticesToManager(GraphManager& manager);
void addVertexToManager(
    GraphManager& manager, std::string vertexName,
    std::vector<std::string> neighbors
);

int main(int argc, char* argv[]) {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Prim's Algorithm Visualizer");
    sf::Font font1;
    GraphManager manager;

    window.setFramerateLimit(60);
    font1.loadFromFile("fonts/Meslo/Meslo LG L Bold Nerd Font Complete.ttf");

    std::string inputText;
    std::string messageInfo;

    bool appLoopRunning = true;
    bool isCalculating = false;
    bool dragging = false;
    bool isReceivingInput = false;
    bool isAddingNewLine = false;
    bool isFirstRun = true;
    bool isGraphFilled = true;
    bool showCursor = true;

    void (*createVertexFromUser)(GraphManager & manager, std::string & inputText) = 
        addVertexFromUser;
    void (*addVerticesToGraph)(GraphManager & manager) = addVerticesToManager;

    Vertex* verticesForNewLine[2] = { nullptr };
    sf::Vector2f tempNewLineCoords[2];
    sf::Vector2f oldMousePos;

    sf::Clock cursorClock;
    sf::Clock delayClock;

    sf::Time elapsed;
    const sf::Time targetFrameTime = sf::seconds(1.f / 2);

    const int userInputTextFont = 28;
    const int messageTextFont = 20;

    sf::Text userInputText;
    userInputText.setPosition(sf::Vector2f(500.f, 650.f));
    userInputText.setFont(font1);
    userInputText.setString("Vertex name: " + inputText);
    userInputText.setCharacterSize(userInputTextFont);
    userInputText.setFillColor(WHITE);

    sf::Text messageText;
    messageText.setPosition(sf::Vector2f(10.f, 650.f));
    messageText.setFont(font1);
    messageText.setCharacterSize(messageTextFont);
    messageText.setFillColor(WHITE);

    sf::Vector2f cursorPos(userInputText.getPosition().x, userInputText.getPosition().y);
    sf::RectangleShape cursor(sf::Vector2f(2.f, 30.f));
    cursor.setFillColor(WHITE);
    cursor.setPosition(cursorPos);

    std::unordered_map<std::string, Vertex* >* vertices = manager.getVerticesMap();
    std::vector<Edge* >* edges = manager.getEdges();
    std::vector<Edge* > MST_Path;
    std::vector<Edge* >::iterator currentEdgeIter;

    std::priority_queue<Edge*, std::vector<Edge*>, EdgeComparator> priorityQueue;
    std::unordered_map<std::string, bool> visited;
    Edge* startEdge = nullptr;

    sf::RectangleShape* movedRect = nullptr;
    sf::RectangleShape* srcRect = nullptr;
    sf::RectangleShape* dstRect = nullptr;
    sf::RectangleShape* srcRectForCalculating = nullptr;
    sf::RectangleShape* dstRectForCalculating = nullptr;

    Button calculateBtn(sf::Vector2f(10.f, 10.f), sf::Vector2f(200.f, 50.f));
    calculateBtn.setButtonColor(GREEN);
    calculateBtn.setButtonText(font1, "Calculate", 24);
    calculateBtn.setButtonTextColor(BLACK);
    calculateBtn.setCallback([&isCalculating, &isFirstRun]() {
        isCalculating = !isCalculating;
        isFirstRun = true;
        });

    Button addVertexBtn(sf::Vector2f(10.f, 70.f), sf::Vector2f(200.f, 50.f));
    addVertexBtn.setButtonColor(BLUE);
    addVertexBtn.setButtonText(font1, "Add Vertex", 24);
    addVertexBtn.setButtonTextColor(WHITE);
    addVertexBtn.setCallback([createVertexFromUser, &manager, &isReceivingInput, &inputText]() {

        if (isReceivingInput) {
            createVertexFromUser(manager, inputText);
        }

        isReceivingInput = !isReceivingInput;
        });

    Button addEdgeBtn(sf::Vector2f(10.f, 130.f), sf::Vector2f(200.f, 50.f));
    addEdgeBtn.setButtonColor(BLUE);
    addEdgeBtn.setButtonText(font1, "Add Edge", 24);
    addEdgeBtn.setButtonTextColor(WHITE);
    addEdgeBtn.setCallback([&isAddingNewLine]() {
        isAddingNewLine = !isAddingNewLine;
        });

    Button clearVerticesBtn(sf::Vector2f(10.f, 210.f), sf::Vector2f(200.f, 50.f));
    clearVerticesBtn.setButtonColor(RED);
    clearVerticesBtn.setButtonText(font1, "Clear Graph", 24);
    clearVerticesBtn.setButtonTextColor(WHITE);
    clearVerticesBtn.setCallback([&isGraphFilled, &manager, &MST_Path]() {
        manager.clearVertices();
        manager.clearEdges();
        MST_Path.clear();
        isGraphFilled = false;
        });

    Button addVerticesBtn(sf::Vector2f(10.f, 270.f), sf::Vector2f(200.f, 50.f));
    addVerticesBtn.setButtonColor(GREEN);
    addVerticesBtn.setButtonText(font1, "Fill Graph", 24);
    addVerticesBtn.setButtonTextColor(WHITE);
    addVerticesBtn.setCallback([addVerticesToGraph, &isGraphFilled, &manager]() {
        if (!isGraphFilled) {
            addVerticesToGraph(manager);
            isGraphFilled = true;
        }
        });

    Button resetBtn(sf::Vector2f(10.f, 350.f), sf::Vector2f(200.f, 50.f));
    resetBtn.setButtonColor(BLUE);
    resetBtn.setButtonText(font1, "Reset", 24);
    resetBtn.setButtonTextColor(WHITE);
    resetBtn.setCallback([&MST_Path]() {
        MST_Path.clear();
        });

    addVerticesToManager(manager);

    while (window.isOpen()) {
        sf::Event event;
        window.clear();

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    for (auto it = vertices->begin(); it != vertices->end(); it++) {
                        Vertex* currentVertex = (*it).second;
                        sf::RectangleShape* currentRect = &currentVertex->vertexRect;
                        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i
                            (event.mouseButton.x, event.mouseButton.y));

                        if (currentRect->getGlobalBounds().contains(mousePos)) {
                            if (isAddingNewLine) {
                                verticesForNewLine[0] = currentVertex;
                                tempNewLineCoords[0] = currentRect->getPosition();
                            } else {
                                oldMousePos = currentRect->getPosition();
                                movedRect = currentRect;
                            }

                            dragging = true;
                        }
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (!isCalculating) {
                        if (isAddingNewLine) {
                            for (auto it = vertices->begin(); it != vertices->end(); it++) {
                                Vertex* currentVertex = (*it).second;
                                sf::RectangleShape* currentRect = &currentVertex->vertexRect;

                                if (currentRect->getGlobalBounds().contains(tempNewLineCoords[1])) {
                                    verticesForNewLine[1] = currentVertex;
                                }
                            }
                        }

                        movedRect = nullptr;
                        dragging = false;
                    }
                }
            }

            if (event.type == sf::Event::MouseMoved) {
                if (dragging && !isCalculating) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i
                        (event.mouseMove.x, event.mouseMove.y));

                    if (isAddingNewLine) {
                        tempNewLineCoords[1] = mousePos;
                    } else {
                        sf::Vector2f difference = mousePos - oldMousePos;
                        movedRect->move(difference);
                        oldMousePos = movedRect->getPosition();
                    }
                }
            }

            if (event.type == sf::Event::TextEntered && isReceivingInput && !isCalculating) {
                if (event.text.unicode < 128) {
                    if (event.text.unicode == '\b' && inputText.length() != 0) {
                        inputText.pop_back();
                    } else if (event.text.unicode != '\r' && event.text.unicode != '\n') {
                        inputText += static_cast<char>(event.text.unicode);
                    }
                }
            }

            calculateBtn.handleEvent(event, window);
            addVertexBtn.handleEvent(event, window);
            addEdgeBtn.handleEvent(event, window);
            clearVerticesBtn.handleEvent(event, window);
            addVerticesBtn.handleEvent(event, window);
            resetBtn.handleEvent(event, window);
        }

        if (isCalculating) {
            if (isFirstRun) {
                isFirstRun = false;
                MST_Path.clear();
                visited.clear();
                
                while (!priorityQueue.empty()) {
                    priorityQueue.pop();
                }

                for (size_t i = 0; i < edges->size(); i++) {
                    std::cout << edges->at(i)->src << " -> " << edges->at(i)->dst << std::endl;
                }

                startEdge = edges->at(0);
                priorityQueue.push(startEdge);
            }

            if (!priorityQueue.empty()) {
                Edge* currentEdge = priorityQueue.top();
                priorityQueue.pop();

                if (!visited[currentEdge->dst]) {
                    visited[currentEdge->dst] = true;

                    if (currentEdge->dst != startEdge->src) {
                        MST_Path.push_back(currentEdge);
                        std::cout << "\nPushed to MST " << currentEdge->src << " -> " << currentEdge->dst << std::endl; 
                    }

                    for (size_t i = 0; i < edges->size(); i++) {
                        Edge* edge = edges->at(i);
                        
                        if (edge->src == currentEdge->dst && !visited[edge->dst]) {
                            priorityQueue.push(edge);
                            std::cout << "Pushed: " << edge->src << " -> " << edge->dst << std::endl;
                        }
                    }
                }
            } else {
                isCalculating = false;

                for (size_t i = 0; i < MST_Path.size(); i++) {
                    Edge* currentEdge = MST_Path.at(i);
                    std::cout << currentEdge->src << " -> " << currentEdge->dst << ": " << 
                        currentEdge->weight << std::endl;
                }
            }
        }

        for (auto it = vertices->begin(); it != vertices->end(); it++) {
            Vertex* currentVertex = (*it).second;
            sf::RectangleShape* vertexRect = &currentVertex->vertexRect;
            sf::Vector2f rectPos(vertexRect->getPosition().x, vertexRect->getPosition().y - 25);
            const int vertexFontSize = 20;

            window.draw(*vertexRect);
            manager.renderText(currentVertex->vertexName, rectPos, font1, vertexFontSize, WHITE, window);

            srcRect = &currentVertex->vertexRect;
            int srcPosX = srcRect->getPosition().x;
            int srcPosY = srcRect->getPosition().y;

            for (std::string& currentNeighbor : currentVertex->neighbors) {
                sf::RectangleShape* neigborRelatedVertexRect = &(*vertices)[currentNeighbor]->vertexRect;
                Edge* currentEdge = manager.getOneEdge(currentVertex->vertexName, currentNeighbor);

                if (!currentEdge) {
                    std::cout << "Error\n";
                    continue;
                }

                std::vector<Edge* >::iterator foundEdge = std::find_if(MST_Path.begin(), MST_Path.end(), [currentEdge](const Edge* resultEdge) {
                    return resultEdge == currentEdge;
                    });

                if (foundEdge == MST_Path.end() && !MST_Path.empty())
                    continue;

                dstRect = neigborRelatedVertexRect;

                const int labelFontSize = 20;
                int dstPosX = dstRect->getPosition().x;
                int dstPosY = dstRect->getPosition().y;
                float length = std::sqrt(std::pow(dstPosX - srcPosX, 2) + std::pow(dstPosY - srcPosY, 2));

                std::string distanceStr = std::to_string(length).substr(0, 6);
                sf::Vector2f labelPos(srcPosX + (dstPosX - srcPosX) / 2, srcPosY + (dstPosY - srcPosY) / 2);
                sf::VertexArray line(sf::Lines, 2);

                currentEdge->weight = length;

                line[0].position = sf::Vector2f(srcPosX, srcPosY);
                line[1].position = sf::Vector2f(dstPosX, dstPosY);

                window.draw(line);
                manager.renderText(distanceStr, labelPos, font1, labelFontSize, WHITE, window);
            }
        }


        if (isAddingNewLine) {
            addEdgeBtn.setButtonText(font1, "Done", 24);

            if (dragging) {
                if (
                    (tempNewLineCoords[0].x != 0 && tempNewLineCoords[0].y != 0) &&
                    (tempNewLineCoords[1].x != 0 && tempNewLineCoords[1].y != 0)
                    ) {
                    sf::VertexArray line(sf::Lines, 2);
                    line[0].position = tempNewLineCoords[0];
                    line[1].position = tempNewLineCoords[1];

                    window.draw(line);
                }
            }
        } else {
            addEdgeBtn.setButtonText(font1, "Add Edge", 24);
        }

        if (verticesForNewLine[0] != nullptr && verticesForNewLine[1] != nullptr) {
            std::string vertex0Name = verticesForNewLine[0]->vertexName;
            std::string vertex1Name = verticesForNewLine[1]->vertexName;

            Edge* foundEdge = manager.getOneEdge(vertex0Name, vertex1Name);

            if (!foundEdge) {
                manager.addEdge(vertex0Name, vertex1Name, 0);
                verticesForNewLine[0]->neighbors.push_back(vertex1Name);
                verticesForNewLine[1]->neighbors.push_back(vertex0Name);
            } else {
                std::cout << "Edge " << vertex0Name << " -> " << vertex1Name << " already created\n";
            }

            verticesForNewLine[0] = nullptr;
            verticesForNewLine[1] = nullptr;

            tempNewLineCoords[0] = sf::Vector2f(0, 0);
            tempNewLineCoords[1] = sf::Vector2f(0, 0);
        }

        if (cursorClock.getElapsedTime().asSeconds() >= 0.5f) {
            showCursor = !showCursor;
            cursorClock.restart();
        }

        if (isReceivingInput) {
            userInputText.setString("Vertex name: " + inputText);

            float lastLetterCursosPos = userInputText.findCharacterPos
                (userInputText.getString().getSize()).x;

            cursorPos.x = lastLetterCursosPos;
            cursor.setPosition(cursorPos);

            addVertexBtn.setButtonText(font1, "Done", 24);
            window.draw(userInputText);

            if (showCursor) {
                window.draw(cursor);
            }
        } else {
            addVertexBtn.setButtonText(font1, "Add Vertex", 24);
            inputText = "";
        }

        if (!isCalculating) {
            window.draw(calculateBtn);
            window.draw(addVertexBtn);
            window.draw(addEdgeBtn);
            window.draw(clearVerticesBtn);
            window.draw(addVerticesBtn);
            window.draw(resetBtn);
        }

        messageText.setString(messageInfo);
        window.draw(messageText);
        window.display();

        if (isCalculating && elapsed < targetFrameTime) {
            sf::sleep(targetFrameTime - elapsed);
        }
    }

    return 0;
}

void addVertexFromUser(GraphManager& manager, std::string& inputText) {
    const int newVertexPosX = 100;
    const int newVertexPosY = 500;

    manager.addVertex(inputText, {}, newVertexPosX, newVertexPosY);
}

void addVerticesToManager(GraphManager& manager) {
    std::vector<std::string> vertexA = {
        "B", "C", "D", "G"
    };

    std::vector<std::string> vertexB = {
        "A", "C", "F", "G"
    };

    std::vector<std::string> vertexC = {
        "A", "B", "D", "E"
    };

    std::vector<std::string> vertexD = {
        "A", "C", "E", "H"
    };

    std::vector<std::string> vertexE = {
        "C", "D", "F"
    };

    std::vector<std::string> vertexF = {
        "E", "B", "H"
    };

    std::vector<std::string> vertexG = {
        "A", "B"
    };

    std::vector<std::string> vertexH = {
        "F", "D"
    };

    manager.addVertex("A", vertexA, 500, 400);
    manager.addVertex("B", vertexB, 540, 80);
    manager.addVertex("C", vertexC, 800, 200);
    manager.addVertex("D", vertexD, 840, 450);
    manager.addVertex("E", vertexE, 900, 175);
    manager.addVertex("F", vertexF, 1000, 100);
    manager.addVertex("G", vertexG, 300, 260);
    manager.addVertex("H", vertexH, 1050, 300);

    std::unordered_map<std::string, std::vector<std::string>> verticesMap = {
        {"A", vertexA}, {"B", vertexB} ,{"C", vertexC}, {"D", vertexD},
        {"E", vertexE}, {"F", vertexF}, {"G", vertexG}, {"H", vertexH}
    };

    for (std::unordered_map<std::string, std::vector<std::string>>::iterator it = verticesMap.begin(); it != verticesMap.end(); it++) {
        const std::pair<std::string, std::vector<std::string>>& currentPair = (*it);

        for (const std::string& currentNeigbor : currentPair.second) {
            Edge* foundEdge = manager.getOneEdge(currentPair.first, currentNeigbor);

            manager.addEdge(currentPair.first, currentNeigbor, 0);
        }
    }

}

void addVertexToManager(
    GraphManager& manager, std::string vertexName,
    std::vector<std::string> neighbors
) {
    manager.addVertex(vertexName, neighbors, 10, 200);
}

