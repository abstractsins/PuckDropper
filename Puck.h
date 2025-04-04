#pragma once
#include <SFML/Graphics.hpp>

class Puck {
public:
    Puck(float radius, sf::Vector2f startPos);
    void draw(sf::RenderWindow& window) const;
    void update(float dt);
    void reset(const sf::Vector2f& position);
    sf::Vector2f getPosition() const { return shape.getPosition(); }
private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
};
