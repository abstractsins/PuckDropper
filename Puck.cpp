#include "Puck.h"
#include "Grid.h"

Puck::Puck(float radius, sf::Vector2f startPos) {
    shape.setRadius(radius);
    shape.setFillColor(sf::Color::White);
    shape.setOrigin(sf::Vector2f{ radius, radius });
    shape.setPosition(startPos);
}

void Puck::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}

void Puck::update(float dt, const Grid& grid, bool enableCollision) {
    const float gravity = 500.f;
    velocity.y += gravity * dt;

    sf::Vector2f prevPos = shape.getPosition();
    sf::Vector2f nextPos = prevPos + velocity * dt;

    if (enableCollision) {
        for (const auto& conn : grid.getConnections()) {
            sf::Vector2f a = grid.getDotPosition(conn.from.x, conn.from.y);
            sf::Vector2f b = grid.getDotPosition(conn.to.x, conn.to.y);
            sf::Vector2f normal;
            if (checkCollisionWithSegment(prevPos, nextPos, a, b, normal)) {
                shape.setPosition(prevPos); // Rewind
                velocity = velocity - 2.f * dot(velocity, normal) * normal; // Reflect
                velocity *= 0.9f; // Dampen
                shape.move(normal * 0.5f); // Nudge out of surface
                break;
            }
        }
    }
    shape.move(velocity * dt);
}

void Puck::reset(const sf::Vector2f& position) {
    velocity = sf::Vector2f(0.f, 0.f);
    shape.setPosition(position);
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
