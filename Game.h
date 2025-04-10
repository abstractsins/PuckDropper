#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include "Grid.h"
#include "Puck.h"
#include "UIManager.h"
#include "Button.h"
#include "Mode.h"

class Game {
public:
    Mode getCurrentMode() { return currentMode; };

    Game();
    void run();

private:
    // Background image for the main menu
    sf::Texture menuBackgroundTexture;
    std::unique_ptr<sf::Sprite> menuBackgroundSprite;
    //sf::Image icon;

    // Starting position for the puck.
    sf::Vector2f startingPos{ 350.f, 80.f };

    // Render window and view
    sf::RenderWindow window;
    sf::View view;

    sf::Font uiFont;
    sf::Font titleFont;

    sf::Music mainMenuMusic;

    // UI buttons for simulation mode.
    Button resetGridButton;
    Button resetPuckButton;
    Button returnToMenuButton;
    Button startButton;
    
    // Free Mode Options
    Button toggleSegmentCollisionButton;
    Button toggleDotCollisionButton;
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
    //enum class Mode { MainMenu, FreeForm, Scoring, About };
    Mode currentMode = Mode::Main;
    bool allowSegmentCollision = false; // In free mode, user can toggle; in scoring mode, always true.
    bool allowDotCollision = false; // In free mode, user can toggle; in scoring mode, always true.
    bool allowPuckBreak = false; // In free mode, user can toggle; in scoring mode, always true.
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
    void initResetGridButton();
    void initResetPuckButton();
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
    void gameReset(std::string resetType = "full");

    bool running = false;
    
    Grid grid;
    Puck puck;
    UIManager uiManager;
    
    int totalScore = 0;
};
