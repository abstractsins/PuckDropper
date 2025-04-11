#pragma once
#include <SFML/Graphics.hpp>
#include "Mode.h"
#include <vector>
#include <optional>
#include <string>

class Grid {

public:

    struct Connection {
        sf::Vector2i from;
        sf::Vector2i to;
        bool isEditable = true;
        sf::Color color = sf::Color::White;
    };

    Grid(int cols, int rows, float spacing, float dotRadius, sf::Vector2f screenSize, float padding = 0.f, Mode mode = Mode::Scoring);
    void draw(sf::RenderWindow& window);
    sf::Vector2f getDotPosition(int col, int row) const;

    void addConnection(int col1, int row1, int col2, int row2);
    bool handleClick(sf::Vector2f mousePos);
    const std::vector<Connection>& getConnections() const;

    int getCols() const { return cols; }
    int getRows() const { return rows; }
    float getDotRadius() const { return dotRadius; }

    void setMode(Mode newMode);

    void resetGrid();

    std::string modeToString(Mode mode) {
        switch (mode) {
            case Mode::Main: return "MainMenu";
            case Mode::Free: return "FreeForm";
            case Mode::Scoring: return "Scoring";
            case Mode::About: return "About";
            default: return "Unknown";
        }
    }
    
    void setFont(const sf::Font& newFont);

    int getSlotIndexFromX(float puckX) const;

    std::vector<int> scoreValues;
    int getNumConnections() { return numConnections; };
    void setNumConnections(int newNum) { numConnections = newNum; };
    bool puckLanded;

private:

    int scoringZoneStart = rows - 3;

    void setBounds(const sf::Vector2f& position, const sf::Vector2f& size);
    std::vector<Connection> connections;
    int cols, rows;
    float spacing, dotRadius;
    sf::Vector2f origin;
    sf::FloatRect gridBounds;
    Mode currentMode;

    std::string scoreTextVertical;

    std::optional<sf::Vector2i> selectedDot;  // None until a dot is clicked
    const Connection* getConnection(sf::Vector2i from, sf::Vector2i to) const;
    void removeConnection(sf::Vector2i from, sf::Vector2i to); // Removes a connection if it already exists

    sf::Font font;

    std::vector<sf::Text> scoreTexts;

    void addBottomBoundary();
    void addSideBoundaries();

    int numConnections = 0;
};