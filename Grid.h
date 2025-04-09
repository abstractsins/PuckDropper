#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>

class Grid {

public:

    struct Connection {
        sf::Vector2i from;
        sf::Vector2i to;
        bool isEditable = true;
        sf::Color color = sf::Color::White;
    };

    Grid(int cols, int rows, float spacing, float dotRadius, sf::Vector2f screenSize, float padding = 0.f);
    void draw(sf::RenderWindow& window);
    sf::Vector2f getDotPosition(int col, int row) const;

    void addConnection(int col1, int row1, int col2, int row2);
    bool handleClick(sf::Vector2f mousePos);
    const std::vector<Connection>& getConnections() const;

    int getCols() const { return cols; }
    int getRows() const { return rows; }
    float getDotRadius() const { return dotRadius; }

    void resetGrid();

private:

    void setBounds(const sf::Vector2f& position, const sf::Vector2f& size);
    std::vector<Connection> connections;
    int cols, rows;
    float spacing, dotRadius;
    sf::Vector2f origin;
    sf::FloatRect gridBounds;

    std::optional<sf::Vector2i> selectedDot;  // None until a dot is clicked
    const Connection* getConnection(sf::Vector2i from, sf::Vector2i to) const;
    void removeConnection(sf::Vector2i from, sf::Vector2i to); // Removes a connection if it already exists
};