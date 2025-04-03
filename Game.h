#pragma once
#pragma once
#include <SFML/Graphics.hpp>
#include "Grid.h"

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update();
    void render();
    void handleResize();

private:
    sf::RenderWindow window;
    sf::View view;
    Grid grid;
};
