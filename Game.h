#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include "Grid.h"
#include "Puck.h"
#include "ScoreEntry.h"
#include "UIManager.h"
#include "Button.h"
#include "Mode.h"

using ScorePair = std::pair<std::string, int>;

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

    sf::Clock runClock;
    float elapsedTime = 0.f;
    bool puckLanded = grid.puckLanded;

    float windowWidth;
    float windowHeight;

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

    void titleCreatedBy();
    void promptToRetry();
    void freeModeOptions();
    void allModesElements();
    void setupMainMenu();
    void displayTopScores();

    // UI initialization functions:
    void initUI();
    void initFont();
    void initUIButtons();
    void initResetGridButton();
    void initResetPuckButton();
    void initReturnToMenuButton();
    void freeModeButtons();
    void onScreenMetrics();
    void initMusicButton();

    // Main Menu UI initialization:
    void initMainMenuUI();
    void setBackground();
    void mainMenuButtons();
    void menuUiContainer();

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
    
    int score = 0;
    int totalScore = 0;
    void scoreCalculator(int score);
    int timeBonusCalculator(int timeMs);
    int timeBonusMultiplier;

    std::string scoreOutputText(int score, int timeBonus, int collisions, int segments, int totalScore);
    std::string filename = "best_scores.txt";
    void displayScore();
    void saveBestScores(const std::vector<ScorePair>& scores, const std::string& filename);
    void updateBestScores(const std::string& playerName, int currentScore, std::vector<ScorePair>& bestScores);
    std::vector<ScorePair> loadBestScores(const std::string& filename);

    void displayTopScoresContainer(const std::vector<ScorePair>& scores);
    void displayBestScores(const std::vector<ScorePair>& scores);

    sf::RectangleShape topScores;

    bool awaitingNameEntry = false;
    bool newHighScoreAchieved = false;
    int pendingHighScore = 0;
    std::string pendingNameInput;
    bool isNewHighScore(int score, const std::vector<ScorePair>& scores);

    void displayRules();
    void displayProjectDetails();
    void aboutTextContainer();
};
