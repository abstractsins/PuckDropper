#include <SFML/Graphics.hpp>
#include "Grid.hpp"
#include <iostream>

int main() {
    sf::Vector2u size{ 800, 600 };
    sf::RenderWindow window(sf::VideoMode{ size, 32 }, "Puck Dropper");

    // Maintain a fixed aspect ratio view
    sf::View view;
    view.setCenter(sf::Vector2f{ 400.f, 300.f });
    view.setSize(sf::Vector2f{ 800.f, 600.f });
    window.setView(view);

    // Grid setup
    const int cols = 7;
    const int rows = 10;
    const float dotRadius = 6.f;
    const float spacing = 60.f;
    Grid grid(cols, rows, spacing, dotRadius, sf::Vector2f{ 800.f, 600.f });

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (event->is<sf::Event::Resized>()) {
                std::cout << "RESIZE\n";

                float screenWidth = 800.f;
                float screenHeight = 600.f;

                sf::Vector2u size = window.getSize();
                float heightRatio = screenHeight / screenWidth;
                float widthRatio = screenWidth / screenHeight;

                if (size.y * widthRatio <= size.x) {
                    size.x = static_cast<unsigned int>(size.y * widthRatio);
                }
                else if (size.x * heightRatio <= size.y) {
                    size.y = static_cast<unsigned int>(size.x * heightRatio);
                }

                window.setSize(size);
            }
        }

        window.clear(sf::Color::Black);
        grid.draw(window);  //  The only line needed to render the whole grid
        window.display();
    }

    return 0;
}
