#include "Puck.h"

Puck::Puck(float radius, sf::Vector2f startPos) {
    shape.setRadius(radius);
    shape.setFillColor(sf::Color::White);
    shape.setOrigin(sf::Vector2f{ radius, radius });
    shape.setPosition(startPos);
}

void Puck::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}

void Puck::update(float dt) {
    const float gravity = 500.f; // units per second^2
    velocity.y += gravity * dt;
    shape.move(velocity * dt);
}

void Puck::reset(const sf::Vector2f& position) {
    velocity = sf::Vector2f(0.f, 0.f); // Reset speed
    shape.setPosition(position);       // Send back to top
}
