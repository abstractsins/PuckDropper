#pragma once
#pragma once
#include <SFML/Graphics.hpp>
#include "Grid.h"
#include "Puck.h"


class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void handleResize();

    sf::RenderWindow window;
    sf::View view;
    sf::Clock clock;
    Grid grid;
    Puck puck;
};
