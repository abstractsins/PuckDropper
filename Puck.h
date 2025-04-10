#pragma once
#include <SFML/Graphics.hpp>
#include "Grid.h"

class Puck {
public:
    Puck(float radius, sf::Vector2f startPos);
    void draw(sf::RenderWindow& window) const;
    void update(
        float dt, 
        const Grid& grid, 
        bool enableSegmentCollision, 
        bool enableDotCollision,
        bool enablePuckBreak
    );
    bool broken() const { return isBroken; }
    void reset(const sf::Vector2f& position);
    sf::Vector2f getPosition() const { return shape.getPosition(); };
    int getCollisions() const { return numCollisions; };
    sf::Vector2f getVelocity() const;

private:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float lastCollisionY = 0.f;    // Y-coordinate at the last collision
    bool isBroken = false;         // Whether the puck is broken

    bool checkCollisionWithSegment(sf::Vector2f a, sf::Vector2f b, sf::Vector2f segA, sf::Vector2f segB, sf::Vector2f & outNormal);
    float dot(sf::Vector2f a, sf::Vector2f b);

    // Helper function to mark the puck as broken.
    void breakPuck() {
        isBroken = true;
        // Change color to give visual feedback; you could also call a shatter effect.
        shape.setFillColor(sf::Color::Red);
    }
    void breakHandler(float lastCollisionY);

    int numCollisions;

};
