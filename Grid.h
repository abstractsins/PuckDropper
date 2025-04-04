#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Grid {
public:
    Grid(int cols, int rows, float spacing, float dotRadius, sf::Vector2f screenSize, float padding = 0.f);
    void draw(sf::RenderWindow& window);
    sf::Vector2f getDotPosition(int col, int row) const;

private:
    int cols, rows;
    float spacing, dotRadius;
    sf::Vector2f origin;
};