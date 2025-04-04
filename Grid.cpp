#include "Grid.h"

Grid::Grid(int cols, int rows, float spacing, float dotRadius, sf::Vector2f screenSize, float padding)
    : cols(cols), rows(rows), spacing(spacing), dotRadius(dotRadius)
{
    float usableWidth = screenSize.x - 2 * padding;
    float usableHeight = screenSize.y - 2 * padding;

    origin.x = padding + (usableWidth - (cols - 1) * spacing) / 2.f;
    origin.y = padding + (usableHeight - (rows - 1) * spacing) / 2.f;
}

sf::Vector2f Grid::getDotPosition(int col, int row) const {
    return {
        origin.x + col * spacing,
        origin.y + row * spacing
    };
}

void Grid::draw(sf::RenderWindow& window) {
    sf::CircleShape dot(dotRadius);
    dot.setOrigin(sf::Vector2f{ dotRadius, dotRadius });
    dot.setFillColor(sf::Color::Cyan);

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            dot.setPosition(getDotPosition(col, row));
            window.draw(dot);
        }
    }
}
