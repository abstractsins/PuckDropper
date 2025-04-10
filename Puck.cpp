#include "Puck.h"
#include "Game.h"
#include "Grid.h"
#include <iostream>
#include <cmath>

//const float FALL_DISTANCE_THRESHOLD = 120.f;
const float FALL_DISTANCE_THRESHOLD = 12000.f; // debug

Puck::Puck(float radius, sf::Vector2f startPos) {
    shape.setRadius(radius);
    shape.setFillColor(sf::Color::White);
    shape.setOrigin(sf::Vector2f{ radius, radius });
    shape.setPosition(startPos);
    numCollisions = 0;
}

void Puck::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}

void Puck::update(
    float dt, 
    const Grid& grid, 
    bool enableSegmentCollision, 
    bool enableDotCollision,
    bool enablePuckBreak
) {
    if (isBroken) return;

    const float gravity = 500.f;
    velocity.y += gravity * dt;

    sf::Vector2f prevPos = shape.getPosition();
    sf::Vector2f nextPos = prevPos + velocity * dt;

    if (enableSegmentCollision) {
        for (const auto& conn : grid.getConnections()) {
            sf::Vector2f a = grid.getDotPosition(conn.from.x, conn.from.y);
            sf::Vector2f b = grid.getDotPosition(conn.to.x, conn.to.y);
            sf::Vector2f normal;
            if (checkCollisionWithSegment(prevPos, nextPos, a, b, normal)) {
                if (!grid.puckLanded) {
                    numCollisions += 1;
                }

                shape.setPosition(prevPos); // Rewind
                velocity = velocity - 2.f * dot(velocity, normal) * normal; // Reflect
                velocity *= 0.85f; // Dampen
                shape.move(normal * 0.5f); // Nudge out of surface
                if (enablePuckBreak) breakHandler(lastCollisionY);
                lastCollisionY = shape.getPosition().y;
                break;
            }
        }
    }

    if (enableDotCollision) {
        bool collided = false;
        // Iterate over grid rows and columns using your new accessors
        for (int row = 0; row < grid.getRows() && !collided; ++row) {
            for (int col = 0; col < grid.getCols() && !collided; ++col) {
                sf::Vector2f dotPos = grid.getDotPosition(col, row);
                float combinedRadius = shape.getRadius() + grid.getDotRadius();
                sf::Vector2f diff = shape.getPosition() - dotPos;
                float distanceSq = diff.x * diff.x + diff.y * diff.y;
                if (distanceSq < combinedRadius * combinedRadius) {

                    // Collision detected with grid dot. Calculate normal.
                    float distance = std::sqrt(distanceSq);
                    sf::Vector2f collisionNormal = (distance > 0.f) ? diff / distance
                        : sf::Vector2f(0.f, -1.f);
                    // Resolve collision by rewinding and reflecting velocity.
                    shape.setPosition(prevPos);
                    velocity = velocity - 2.f * dot(velocity, collisionNormal) * collisionNormal;
                    velocity *= 0.85f; // Dampen the velocity a bit
                    shape.move(collisionNormal * 0.5f); // Nudge out of the dot area
                    collided = true;
                    if (!grid.puckLanded) {
                        numCollisions += 1;
                    }
                    if (enablePuckBreak) breakHandler(lastCollisionY);
                    lastCollisionY = shape.getPosition().y;
                }
            }
        }
    }

    shape.move(velocity * dt);
}


void Puck::breakHandler(float lastCollisionY) {
    float fallDistance = shape.getPosition().y - lastCollisionY;
    if (fallDistance > FALL_DISTANCE_THRESHOLD) {
        breakPuck();
    }
}

void Puck::reset(const sf::Vector2f& position) {
    velocity = sf::Vector2f(0.f, 0.f);
    shape.setPosition(position);
    isBroken = false;
    shape.setFillColor(sf::Color::White);  // Reset to normal color
    lastCollisionY = position.y;
    numCollisions = 0;
}

float Puck::dot(sf::Vector2f a, sf::Vector2f b) {
    return a.x * b.x + a.y * b.y;
}

bool Puck::checkCollisionWithSegment(
    sf::Vector2f prevPos,
    sf::Vector2f nextPos,
    sf::Vector2f segA,
    sf::Vector2f segB,
    sf::Vector2f& normal
) {
    sf::Vector2f ab = segB - segA;
    sf::Vector2f ap = shape.getPosition() - segA;
    float abLenSquared = ab.x * ab.x + ab.y * ab.y;
    float t = std::max(0.f, std::min(1.f, (ap.x * ab.x + ap.y * ab.y) / abLenSquared));
    sf::Vector2f closestPoint = segA + t * ab;
    sf::Vector2f diff = shape.getPosition() - closestPoint;
    float distSq = diff.x * diff.x + diff.y * diff.y;
    float radius = shape.getRadius();
    if (distSq <= radius * radius) {
        float len = std::sqrt(distSq);
        if (len != 0.f)
            normal = diff / len;
        else
            normal = { 0.f, -1.f }; // arbitrary direction
        return true;
    }
    return false;
}

static float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
    return a.x * b.x + a.y * b.y;
}

sf::Vector2f Puck::getVelocity() const {
    return velocity;
}
