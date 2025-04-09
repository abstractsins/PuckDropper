#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include "Grid.h"
#include "Puck.h"
#include "UIManager.h"
#include "Button.h"

class Game {
public:
    Game();
    void run();

private:
    // Background image for the main menu
    sf::Texture menuBackgroundTexture;
    std::unique_ptr<sf::Sprite> menuBackgroundSprite;

    // Starting position for the puck.
    sf::Vector2f startingPos{ 350.f, 80.f };

    // Render window and view
    sf::RenderWindow window;
    sf::View view;

    sf::Font uiFont;
    sf::Font titleFont;

    sf::Music mainMenuMusic;

    // UI buttons for simulation mode.
    Button resetButton;
    Button returnToMenuButton;
    Button startButton;
    
    // Free Mode Options
    Button togglePuckCollisionButton;
    Button togglePuckBreakButton;

    sf::RectangleShape menuContainer;

    // UI buttons for main menu.
    Button freeModeButton;
    Button scoringModeButton;
    Button rulesModeButton;

    // Utility Buttons
    Button musicButton;

    sf::Clock clock;

    // Mode selection:
    enum class Mode { MainMenu, FreeForm, Scoring, About };
    Mode currentMode = Mode::MainMenu;
    bool allowPuckCollision = false; // In free mode, user can toggle; in scoring mode, always true.
    bool allowPuckBreak = false;
    bool simulationStarted = false;

    void processEvents();
    void handleResize();
    void update(float dt);
    bool isPuckOutOfBounds();
    void render();
    
    // UI initialization functions:
    void initUI();
    void initFont();
    void initUIButtons();
    void initResetButton();
    void initReturnToMenuButton();

    // Main Menu UI initialization:
    void initMainMenuUI();

    // Mode switching functions:
    void enterFreeFormMode();
    void enterScoringMode();
    void enterMainMenu();
    void enterAboutMode();

    void startSimulation();

    void handleMouseClick(const sf::Vector2f& pos);
    void gameReset();
    
    Grid grid;
    Puck puck;
    UIManager uiManager;
};
