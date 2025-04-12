#include "Game.h"
#include "UIManager.h"
#include "Colors.h"
#include "Mode.h"
#include "ScoreEntry.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include <algorithm>

//
// SOUNDS
// 
// mode switch
sf::SoundBuffer modeStartBuffer("modeStart.ogg");
sf::Sound modeStartSound(modeStartBuffer);
// puck land
sf::SoundBuffer puckLandedBuffer("puckLanded.ogg");
sf::Sound puckLandedSound(puckLandedBuffer);

Game::Game()
	: window(sf::VideoMode({ 800, 800 }), "Puck Dropper"),
	view({ 400.f, 300.f }, sf::Vector2f{ 800.f, 800.f }),
	grid(14, 20, 30.f, 1.5, sf::Vector2f{ 800.f, 800.f }, 0.f, Mode::Scoring),
	puck(7.f, startingPos),
	mainMenuMusic("symphbass.wav")
{
	sf::Image icon;
	initFont();
	if (!icon.loadFromFile("puck.png")) {
		std::cout << "icon not found\n\n";
	}
	// Set the icon: width, height, and pointer to pixels
	window.setIcon({ icon.getSize().x, icon.getSize().y }, icon.getPixelsPtr());
	// Start in Main Menu mode.
	initMainMenuUI();
}

void Game::run() {
	const float fixedDt = 1.f / 60.f; // 60 FPS timestep
	float accumulator = 0.f;

	sf::Clock frameClock;

	while (window.isOpen()) {
		float dt = frameClock.restart().asSeconds();
		if (dt > 0.25f) dt = 0.25f; // clamp if system stalls or window is moved
		accumulator += dt;

		processEvents();

		while (accumulator >= fixedDt) {
			// Physics update
			if (currentMode != Mode::Main && currentMode != Mode::About && simulationStarted) {
				puck.update(fixedDt, grid, allowSegmentCollision, allowDotCollision, allowPuckBreak, puckLanded);
				if (isPuckOutOfBounds()) {
					puck.reset(startingPos);
					runClock.restart();
				}
			}

			// Scoring logic (only once per frame still fine)
			if (currentMode == Mode::Scoring && !puckLanded) {
				sf::Vector2f puckPos = puck.getPosition();
				sf::Vector2f velocity = puck.getVelocity();

				if (puckPos.y > grid.getDotPosition(0, grid.getRows() - 3).y) {
					if (std::abs(velocity.x) < 3 && std::abs(velocity.y) < 2) {
						puckLanded = true;
						puck.setFillColor(sf::Color::Green);
						puckLandedSound.setVolume(50.f);
						puckLandedSound.play();
						elapsedTime = runClock.getElapsedTime().asSeconds();

						int col = grid.getSlotIndexFromX(puckPos.x);
						std::cout << "Puck landed in column: " << col << "\n";

						score = grid.scoreValues[col];
						scoreCalculator(score);

						auto bestScores = loadBestScores(filename);
						if (isNewHighScore(totalScore, bestScores)) {
							awaitingNameEntry = true;
							newHighScoreAchieved = true;
							pendingHighScore = totalScore;
							pendingNameInput.clear();
						}
						else {
							updateBestScores("Hot Nickels", totalScore, bestScores);
						}
					}
				}
			}

			if (puck.broken()) {
				runClock.stop();
			}

			accumulator -= fixedDt;
		}

		render();
	}
}


void Game::processEvents() {
	uiManager.update(window);

	while (auto event = window.pollEvent()) {

		if (awaitingNameEntry) {
			if (event->is<sf::Event::TextEntered>()) {
				auto typed = event->getIf<sf::Event::TextEntered>();
				if (typed) {
					if (typed->unicode == '' && !pendingNameInput.empty()) {
						pendingNameInput.pop_back();
					}
					else if (typed->unicode == '\r' || typed->unicode == '\n') {
						if (pendingNameInput.size() == 0) {
							pendingNameInput += "???";
						}
						auto bestScores = loadBestScores(filename);
						updateBestScores(pendingNameInput, pendingHighScore, bestScores);
						awaitingNameEntry = false;
					}
					else if (typed->unicode < 128 && std::isprint(typed->unicode)) {
						if (pendingNameInput.size() < 12)
							pendingNameInput += static_cast<char>(typed->unicode);
					}
				}
			}
			return; // skip normal game input
		}

		if (event->is<sf::Event::Closed>())
			window.close();

		if (event->is<sf::Event::Resized>()) {
			handleResize();
		}

		if (event->is<sf::Event::KeyPressed>()) {
			if (puck.broken() || puckLanded) {
				gameReset("soft");  // Reset the puck and grid as defined in Game::gameReset() 
				continue;
			}
			else {
				if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
					// If Enter is pressed, simulate a start button click.
					if (keyEvent->code == sf::Keyboard::Key::Enter) {
						running ? gameReset("soft") : startButton.onClick();
					}
				}
			}
		}

		if (event->is<sf::Event::MouseButtonPressed>()) {
			sf::Vector2i pixel = sf::Mouse::getPosition(window);
			sf::Vector2f worldPos = window.mapPixelToCoords(pixel, window.getView());
			handleMouseClick(worldPos);
		}

		uiManager.handleEvent(*event, window);
	}
}


void Game::handleMouseClick(const sf::Vector2f& pos) {
	std::cout << "\nMouse world pos: (" << pos.x << ", " << pos.y << ")\n";

	if (uiManager.handleClick(pos)) {
		return;
	} 
	else if (currentMode == Mode::Scoring && running) {
		 //Don’t allow editing during scoring simulation
		return; // off for debug
	} 
	else if (grid.handleClick(pos)) {
		//std::cout << "grid called\n";
		return;
	}
}

void Game::handleResize() {
	// set screen size
	float screenWidth = 800.f;
	float screenHeight = 800.f;

	// get the resized size
	sf::Vector2u size = window.getSize();

	// setup my wanted aspect ratio
	float  heightRatio = screenHeight / screenWidth;
	float  widthRatio = screenWidth / screenHeight;

	// adapt the resized window to my wanted aspect ratio
	if (size.y * widthRatio <= size.x) {
		size.x = size.y * widthRatio;
	}
	else if (size.x * heightRatio <= size.y) {
		size.y = size.x * heightRatio;
	}
	// set the new size
	window.setSize(size);
}

void Game::update(float dt) {
	// If in simulation mode, update simulation UI elements (e.g. toggle button for free form).
	if (currentMode != Mode::About && currentMode != Mode::Main) {
		// For Free Form mode, update the toggle button appearance.
		if (currentMode == Mode::Free) {
			toggleSegmentCollisionButton.update(window);
		}
	}
}

bool Game::isPuckOutOfBounds() {
	// Reset if the puck falls out of view
	const float screenBottom = 800.f;
	const float screenLeft = 0.f;
	const float screenRight = 800.f;
	if (
		puck.getPosition().y > screenBottom + 50.f
		|| puck.getPosition().x > screenRight
		|| puck.getPosition().x < screenLeft ) {
		return true;
	}
	return false;
}


void Game::render() {
	window.clear(sf::Color::Black);

	if (currentMode == Mode::Main || currentMode == Mode::About) {
		// Draw the background image first if available.
		if (menuBackgroundSprite) {
			window.draw(*menuBackgroundSprite);
		}
		initMusicButton();
		titleCreatedBy();
		if (puck.getPuckIsStill() && !puckLanded && running) {
			puck.breakPuck();
		}
	}
	else {
		allModesElements();

		if (currentMode == Mode::Free) {
			freeModeOptions();
		}

		if (currentMode == Mode::Scoring) {
			auto scores = loadBestScores(filename);
			displayTopScoresContainer(scores);
		}

		if (puck.broken()) {
			promptToRetry();
		} 
		else if (puckLanded) {
			displayScore();
			promptToRetry();
		}
	}

	if (currentMode == Mode::Main) {
		setupMainMenu();
	} 
	else if (currentMode == Mode::About) {
		returnToMenuButton.draw(window);
		aboutTextContainer();
		displayRules();
		displayProjectDetails();
		// display rules
	}

	if (awaitingNameEntry) {
		// Box backdrop
		sf::RectangleShape promptContainer({ 350.f, 180.f });
		promptContainer.setFillColor(sf::Color(0, 0, 0));
		promptContainer.setOutlineColor(sf::Color::Yellow);
		promptContainer.setOutlineThickness(2.f);
		promptContainer.setOrigin(promptContainer.getSize() / 2.f);
		promptContainer.setPosition({ 450.f, 400.f });  // center of window

		// Prompt (static part)
		sf::Text promptText(titleFont, "New High Score!\nEnter your name:", 24);
		promptText.setFillColor(sf::Color::White);
		promptText.setOutlineColor(sf::Color::Black);
		promptText.setOutlineThickness(1.f);

		// Center horizontally in container
		sf::FloatRect textBounds = promptText.getLocalBounds();
		promptText.setOrigin({textBounds.size.x / 2.f, 0.f});  // center X only
		promptText.setPosition({ promptContainer.getPosition().x, promptContainer.getPosition().y - 75.f });

		// Input (dynamic part)
		sf::Text userInputText(uiFont, pendingNameInput + "_", 28);
		userInputText.setFillColor(sf::Color::Cyan);
		userInputText.setOutlineColor(sf::Color::Black);
		userInputText.setOutlineThickness(1.f);
		userInputText.setStyle(sf::Text::Bold);
		userInputText.setPosition({ promptContainer.getPosition().x - 60.f, promptContainer.getPosition().y + 25.f });

		// Draw all
		window.draw(promptContainer);
		window.draw(promptText);
		window.draw(userInputText);
		window.display();
		return;
	}

	window.display();
}

void Game::displayTopScoresContainer(const std::vector<ScorePair>& scores) {
	topScores.setSize({ 250.f, 350.f });
	topScores.setFillColor(sf::Color::Transparent);
	topScores.setOutlineColor(sf::Color::Yellow); 
	topScores.setOutlineThickness(1.f); 
	topScores.setPosition({ 20.f, 150.f }); 

	sf::Text topScoresTitle(titleFont, "Top Scores", 24);
	topScoresTitle.setFillColor(sf::Color::Yellow);

	sf::FloatRect textBounds = topScoresTitle.getLocalBounds();
	sf::Vector2f size = textBounds.size;
	topScoresTitle.setOrigin(size / 2.f);
	topScoresTitle.setPosition({ topScores.getPosition().x + topScores.getSize().x / 2.f, topScores.getPosition().y + 10.f });

	window.draw(topScores); 
	window.draw(topScoresTitle);
	
	displayBestScores(scores);
}

void Game::setupMainMenu() {
	// Then draw the main menu buttons.
	window.draw(menuContainer);
	freeModeButton.draw(window);
	scoringModeButton.draw(window);
	rulesModeButton.draw(window);
}

void Game::allModesElements() {
	// Simulation mode drawing.
	grid.draw(window);
	puck.draw(window);
	resetGridButton.draw(window);
	resetPuckButton.draw(window);
	startButton.draw(window);
	onScreenMetrics();
	returnToMenuButton.draw(window);
}

void Game::freeModeOptions() {
	toggleSegmentCollisionButton.draw(window);
	toggleDotCollisionButton.draw(window);
	togglePuckBreakButton.draw(window);
}

void Game::promptToRetry() {
	// Reset Prompt By
	sf::Text userResetPrompt(uiFont, "Press Any Key to Reset Puck", 24);
	userResetPrompt.setPosition({ 300.f, 50.f });
	userResetPrompt.setFillColor(Colors::Whitesmoke);
	userResetPrompt.setOutlineColor(sf::Color::Green);
	userResetPrompt.setOutlineThickness(0.25);
	window.draw(userResetPrompt);
}

void Game::onScreenMetrics() {
	// collision counter
	int numCollisions = puck.getCollisions();
	sf::Text collisionScore(uiFont, "Collisions: " + std::to_string(numCollisions), 18);
	collisionScore.setPosition({ 325.f, 10.f });
	collisionScore.setFillColor(Colors::Whitesmoke);
	window.draw(collisionScore);

	// run clock
	float displayTime = puckLanded ? elapsedTime : runClock.getElapsedTime().asSeconds();
	sf::Text elapsedTimeText(uiFont, "Time: " + std::to_string(displayTime), 18);
	elapsedTimeText.setPosition({ 500.f, 10.f });
	elapsedTimeText.setFillColor(Colors::Whitesmoke);
	window.draw(elapsedTimeText);

	// num connections
	sf::Text connectionsText(uiFont, "Connections: " + std::to_string(grid.getNumConnections()), 18);
	connectionsText.setPosition({ 325.f, 30.f });
	connectionsText.setFillColor(Colors::Whitesmoke);
	window.draw(connectionsText);
}

void Game::titleCreatedBy() {
	// Title
	sf::Text titleText(titleFont, "Puck Dropper", 86);
	titleText.setPosition({ 100.f, 50.f });
	titleText.setFillColor(sf::Color::Yellow);
	titleText.setOutlineColor(sf::Color::Black);
	titleText.setOutlineThickness(0.5);
	window.draw(titleText);

	// Created By
	sf::Text createdByText(uiFont, "Created by Daniel Berlin, 2025", 16);
	createdByText.setPosition({ 550.f, 750.f });
	createdByText.setFillColor(Colors::Whitesmoke);
	createdByText.setOutlineColor(sf::Color::Black);
	createdByText.setOutlineThickness(0.5);
	window.draw(createdByText);
}

//
// UI initialization functions:
//
void Game::initUI() {
	initFont();
	initUIButtons();
}

void Game::initFont() {
	if (!uiFont.openFromFile("Tuffy.ttf")) {
		std::cerr << "Failed to open Tuffy.ttf\n";
	}
	if (!titleFont.openFromFile("RubikGlitch.ttf")) {
		std::cerr << "Failed to open RubikGlitch.ttf\n";
	}
	grid.setFont(uiFont);
}

void Game::initUIButtons() {
	// Initialize simulation UI buttons.
	initResetGridButton();
	initResetPuckButton();
	// (Existing toggle button for FreeForm mode gets added only if applicable.)
	if (currentMode == Mode::Free) {
		freeModeButtons();
	}
	initReturnToMenuButton();

	startButton = Button(sf::Vector2f(120.f, 40.f), "Start", uiFont, [this]() {
		startSimulation();
		runClock.restart();
		running = true;
		startButton.setText("Running...");
		startButton.shape.setFillColor(Colors::ForestGreen);
	});

	// Place the start button at an appropriate position (for example, next to Reset).
	startButton.setPosition(sf::Vector2f(20.f, 70.f));
	uiManager.addButton(startButton);
}

void Game::freeModeButtons() {
	toggleSegmentCollisionButton = Button(sf::Vector2f(150.f, 40.f), "Line Collision: OFF", uiFont, [this]() {
		toggleSegmentCollisionButton.toggled = !toggleSegmentCollisionButton.toggled;
		allowSegmentCollision = toggleSegmentCollisionButton.toggled; // mirror that externally
		std::string text = toggleSegmentCollisionButton.toggled ? "Line Collision: ON" : "Line Collision: OFF";
		toggleSegmentCollisionButton.setText(text);
		// Optionally update appearance here:
		toggleSegmentCollisionButton.shape.setFillColor(toggleSegmentCollisionButton.toggled ? Colors::LemonYellow : Colors::Whitesmoke);
	});
	toggleSegmentCollisionButton.setPosition(sf::Vector2f(20.f, 150.f));
	uiManager.addButton(toggleSegmentCollisionButton);

	toggleDotCollisionButton = Button(sf::Vector2f(150.f, 40.f), "Dot Collision: OFF", uiFont, [this]() {
		toggleDotCollisionButton.toggled = !toggleDotCollisionButton.toggled;
		allowDotCollision = toggleDotCollisionButton.toggled; // mirror that externally
		std::string text = toggleDotCollisionButton.toggled ? "Dot Collision: ON" : "Dot Collision: OFF";
		toggleDotCollisionButton.setText(text);
		// Optionally update appearance here:
		toggleDotCollisionButton.shape.setFillColor(toggleDotCollisionButton.toggled ? Colors::LemonYellow : Colors::Whitesmoke);
	});
	toggleDotCollisionButton.setPosition(sf::Vector2f(20.f, 200.f));
	uiManager.addButton(toggleDotCollisionButton);

	togglePuckBreakButton = Button(sf::Vector2(150.f, 40.f), "Puck Break: OFF", uiFont, [this]() {
		togglePuckBreakButton.toggled = !togglePuckBreakButton.toggled;
		allowPuckBreak = togglePuckBreakButton.toggled; // mirror that externally
		std::string text = togglePuckBreakButton.toggled ? "Puck Break: ON" : "Puck Break: OFF";
		togglePuckBreakButton.setText(text);
		// Optionally update appearance here:
		togglePuckBreakButton.shape.setFillColor(togglePuckBreakButton.toggled ? Colors::LemonYellow : Colors::Whitesmoke);
	});
	togglePuckBreakButton.setPosition(sf::Vector2f(20.f, 250.f));
	uiManager.addButton(togglePuckBreakButton);
}

void Game::initResetGridButton() {
	// Create a new reset button using our Button class.
	// The callback here resets the puck when the button is clicked.
	resetGridButton = Button(sf::Vector2f(120.f, 40.f), "Reset Grid", uiFont, [this]() {
		gameReset("full");
	});
	resetGridButton.setPosition(sf::Vector2f(20.f, 20.f));
	resetGridButton.setFillColor(Colors::LightRed);
	uiManager.addButton(resetGridButton);
}

void Game::initResetPuckButton() {
	// Create a new reset button using our Button class.
	// The callback here resets the puck when the button is clicked.
	resetPuckButton = Button(sf::Vector2f(120.f, 40.f), "Reset Puck", uiFont, [this]() {
		gameReset("soft");
	});
	resetPuckButton.setPosition(sf::Vector2f(150.f, 20.f));
	resetPuckButton.setFillColor(Colors::LightRed);
	uiManager.addButton(resetPuckButton);
}

void Game::initReturnToMenuButton() {
	returnToMenuButton = Button(sf::Vector2f(100.f, 40.f), "Menu", uiFont, [this]() {
		enterMainMenu();
	});
	returnToMenuButton.setPosition(sf::Vector2f(20.f, 720.f));
	uiManager.addButton(returnToMenuButton);
}

//
// Main Menu UI initialization:
//
void Game::initMainMenuUI() {
	mainMenuMusic.setLooping(true);
	mainMenuMusic.play();

	windowWidth = static_cast<float>(window.getSize().x);
	windowHeight = static_cast<float>(window.getSize().y);

	// Clear existing buttons from the UI.
	uiManager.clearButtons();

	menuUiContainer();
	setBackground();
	mainMenuButtons();

}

void Game::menuUiContainer() {
	// --- CONTAINER SETUP ---
	// Decide how big you want the container
	float containerWidth = 300.f;
	float containerHeight = 300.f;
	menuContainer.setSize(sf::Vector2f({ containerWidth, containerHeight }));

	// Fill color (semi-transparent dark overlay). Adjust RGBA to your preference.
	menuContainer.setFillColor(sf::Color(0, 0, 0, 150));
	// Optional: Outline color and thickness
	menuContainer.setOutlineThickness(2.f);
	menuContainer.setOutlineColor(sf::Color::White);

	// Center the container. We set the shape's origin to its midpoint,
	// and then position it at the window center.
	menuContainer.setOrigin({ containerWidth / 2.f, containerHeight / 2.f });
	//menuContainer.setPosition({ windowWidth / 2.f, windowHeight / 2.f });
	menuContainer.setPosition({ 400.f, 400.f });
}

void Game::setBackground() {

	// Load the background image (menuBG.jpg).
	if (!menuBackgroundTexture.loadFromFile("menuBG.jpg")) {
		std::cerr << "Error loading menuBG.jpg" << std::endl;
	}

	// Construct the sprite using the loaded texture.
	menuBackgroundSprite = std::make_unique<sf::Sprite>(menuBackgroundTexture);
	menuBackgroundSprite->setColor(sf::Color(255, 255, 255, 100));

	// Obtain the local bounds of the sprite.
	sf::FloatRect spriteBounds = menuBackgroundSprite->getLocalBounds();

	// Calculate the scale factor so that the sprite's width becomes exactly the window's width.
	float scaleFactor = windowWidth / spriteBounds.size.x;

	// Apply the uniform scale so that the sprite's width matches 100% of the window.
	menuBackgroundSprite->setScale({ scaleFactor, scaleFactor });

	// Optionally, center the sprite vertically if its scaled height is less than the window height.
	float scaledHeight = spriteBounds.size.y * scaleFactor;
	float yOffset = (windowHeight - scaledHeight) / 2.f;
	menuBackgroundSprite->setPosition({ 0.f, yOffset });

}

void Game::mainMenuButtons() {
	// Initialize main menu buttons with updated naming conventions:
	freeModeButton = Button(sf::Vector2f(200.f, 50.f), "Free Mode", uiFont, [this]() {
		enterFreeFormMode();
		});
	freeModeButton.setPosition(sf::Vector2f(300.f, 300.f));
	uiManager.addButton(freeModeButton);

	scoringModeButton = Button(sf::Vector2f(200.f, 50.f), "Scoring Mode", uiFont, [this]() {
		enterScoringMode();
		});
	scoringModeButton.setPosition(sf::Vector2f(300.f, 375.f));
	uiManager.addButton(scoringModeButton);

	rulesModeButton = Button(sf::Vector2f(200.f, 50.f), "About", uiFont, [this]() {
		enterAboutMode();
		});
	rulesModeButton.setPosition(sf::Vector2f(300.f, 450.f));
	uiManager.addButton(rulesModeButton);

	musicButton = Button(sf::Vector2f(150.f, 50.f), "Music On", uiFont, [this]() {
		musicButton.toggled = !musicButton.toggled;
		musicButton.setFillColor(musicButton.toggled ? Colors::Whitesmoke : Colors::LemonYellow);
		musicButton.toggled ? mainMenuMusic.setVolume(0.f) : mainMenuMusic.setVolume(100.f);
		std::string text = musicButton.toggled ? "Music Off" : "Music On";
		musicButton.setText(text);
	});
	musicButton.setPosition(sf::Vector2f(575.f, 650.f));
	uiManager.addButton(musicButton);
}



//
// Mode switching:
//
void Game::enterFreeFormMode() {
	mainMenuMusic.stop();
	modeStartSound.setVolume(20.f);
	modeStartSound.play();
	// Remove main menu buttons.
	uiManager.clearButtons();
	currentMode = Mode::Free;
	grid.setMode(Mode::Free);
	allowSegmentCollision = false;  // Default for free form; user can toggle.
	allowDotCollision = false;  // Default for free form; user can toggle.
	allowPuckBreak = false;  // Default for free form; user can toggle.
	initUIButtons();
	gameReset("full");
}

void Game::enterScoringMode() {
	mainMenuMusic.stop();
	modeStartSound.setVolume(20.f);
	modeStartSound.play();
	uiManager.clearButtons();
	currentMode = Mode::Scoring;
	grid.setMode(Mode::Scoring);
	allowSegmentCollision = true;  // Always collide in scoring mode.
	allowDotCollision = true; // Always collide in scoring mode.
	allowPuckBreak = true;
	initUIButtons();
	gameReset("full");
}

void Game::enterAboutMode() {
	uiManager.clearButtons();
	currentMode = Mode::About;
	initUIButtons();
	uiManager.addButton(musicButton);
}

void Game::enterMainMenu() {
	uiManager.clearButtons();
	currentMode = Mode::Main;
	initMainMenuUI();
	grid.setNumConnections(0);
}

void Game::gameReset(std::string resetType) {
	puck.reset(startingPos);
	simulationStarted = false;
	running = false;
	puckLanded = false;
	startButton.setText("Start");
	startButton.shape.setFillColor(Colors::LightGreen);
	totalScore = 0;
	runClock.reset();
	if (resetType == "full") {
		grid.resetGrid();
	}
}

void Game::startSimulation() {
	simulationStarted = true;
}

// 
// SCORING
//
void Game::scoreCalculator(int score) {
	totalScore += score;
	int time = runClock.getElapsedTime().asMilliseconds();
	timeBonusMultiplier = timeBonusCalculator(time);
	totalScore *= timeBonusMultiplier;
	totalScore -= grid.getNumConnections();
	totalScore -= puck.getCollisions() * 4;
	//std::cout << "time: " << std::to_string(time) << "ms\n\n";
	//std::cout << "timeBonusPoints: " << std::to_string(timeBonusMultiplier) << "\n\n";
	//std::cout << std::to_string(totalScore) << "\n\n";
}

int Game::timeBonusCalculator(int timeMs) {
	if (timeMs < 10000) return 7;
	else if (timeMs < 15000) return 6;
	else if (timeMs < 20000) return 5;
	else if (timeMs < 25000) return 4;
	else if (timeMs < 30000) return 3;
	else return 1;
}

std::string Game::scoreOutputText(int score, int timeBonus, int collisions, int segments, int totalScore) {
	std::string output;

	output += "Score: " + std::to_string(score) + "...\n";
	output += "Time Bonus: x" + std::to_string(timeBonus) + "...\n";
	output += "Collisions Penalty: -" + std::to_string(collisions) + "...\n";
	output += "Connections Penalty: -" + std::to_string(segments * 4) + "...\n\n";
	output += "Total Score: " + std::to_string(totalScore);

	return output;
}

void Game::displayScore() {
	sf::RectangleShape scoreReadoutArea({ 250.f, 175.f });

	scoreReadoutArea.setPosition({ 20.f, 510.f });
	scoreReadoutArea.setFillColor(sf::Color::Transparent);
	scoreReadoutArea.setOutlineColor(sf::Color::Cyan);
	scoreReadoutArea.setOutlineThickness(1.f);

	std::string scoreReadout = scoreOutputText(score, timeBonusMultiplier, puck.getCollisions(), grid.getNumConnections(), totalScore);
	sf::Text scoreReadoutText(uiFont, scoreReadout, 18);
	//std::cout << scoreReadoutText.getString().toAnsiString() << "\n";
	float padding = 20.f;
	scoreReadoutText.setPosition({ scoreReadoutArea.getPosition().x + padding, scoreReadoutArea.getPosition().y + padding });

	window.draw(scoreReadoutArea);
	window.draw(scoreReadoutText);
}


void Game::updateBestScores(const std::string& playerName, int currentScore, std::vector<ScorePair>& bestScores) {
	// Add the new entry
	bestScores.push_back({ playerName, currentScore });

	// Sort in descending order (highest scores first)
	std::sort(bestScores.begin(), bestScores.end(), [](const ScorePair& a, const ScorePair& b) {
		return a.second > b.second;
		});

	// Optionally, keep only the top 10 scores
	if (bestScores.size() > 10) {
		bestScores.resize(10);
	}

	// Save the updated scores
	saveBestScores(bestScores, filename);
}

bool Game::isNewHighScore(int score, const std::vector<ScorePair>& scores) {
	if (scores.size() < 10) return true;
	return score > scores.back().second;
}

void Game::displayBestScores(const std::vector<ScorePair>& scores) {
	std::string scoreListString;
	for (int i = 0; i < scores.size(); ++i) {
		const std::string& name = scores[i].first;
		int score = scores[i].second;

		scoreListString += std::to_string(i + 1); // index starts from 1
		scoreListString += ": ";
		scoreListString += name;
		scoreListString += " - ";
		scoreListString += std::to_string(score); // <-- fix: convert int to string
		scoreListString += '\n';
	}

	sf::Text scoreListText(uiFont, scoreListString, 20);
	scoreListText.setFillColor(sf::Color::White);

	scoreListText.setPosition(topScores.getPosition() + sf::Vector2f(20.f, 40.f)); // indent slightly under title
	window.draw(scoreListText);
}


void Game::saveBestScores(const std::vector<ScorePair>& scores, const std::string& filename) {
	std::ofstream outFile(filename);
	if (!outFile) {
		std::cerr << "Error opening file " << filename << " for writing.\n";
		return;
	}

	// Write each pair as "name,score" on a separate line.
	for (const auto& scoreEntry : scores) {
		outFile << scoreEntry.first << "," << scoreEntry.second << "\n";
	}
	// The file is closed automatically when outFile goes out of scope.
}

std::vector<ScorePair> Game::loadBestScores(const std::string& filename) {
	std::vector<ScorePair> scores;
	std::ifstream inFile(filename);
	if (!inFile) {
		std::cerr << "File not found: " << filename << ". Returning an empty score list.\n";
		return scores;  // Return an empty vector if the file doesn’t exist.
	}

	std::string line;
	while (std::getline(inFile, line)) {
		std::istringstream lineStream(line);
		std::string name;
		std::string scoreStr;

		// Extract the name and score (using comma as delimiter)
		if (std::getline(lineStream, name, ',') && std::getline(lineStream, scoreStr)) {
			int score = std::stoi(scoreStr);
			scores.push_back({ name, score });
		}
	}

	return scores;
}

void Game::aboutTextContainer() {
	windowWidth = static_cast<float>(window.getSize().x);
	windowHeight = static_cast<float>(window.getSize().y);
	sf::RectangleShape container({ 700.f, 460.f });
	container.setFillColor(sf::Color(0, 0, 0, 150));
	container.setOutlineColor(Colors::Whitesmoke);
	container.setOutlineThickness(3.f);
	container.setOrigin({ container.getSize().x / 2.f, container.getSize().y / 2.f });
	container.setPosition({ window.getSize().x / 2.f, window.getSize().x / 2.f });
	window.draw(container);
}

void Game::displayRules() {
	windowWidth = static_cast<float>(window.getSize().x);
	windowHeight = static_cast<float>(window.getSize().y);

	std::string rulesString;
	rulesString += "Drop the puck and land it in a high-value bucket using as few segments as possible.\n\n";
	rulesString += "Click between dots to build segments. The puck bounces off segments and dots.\n";
	rulesString += "If the puck falls too far without a bounce, it breaks.\n\n";
	rulesString += " Bonus for time. Penalties for segments and collisions.\n";

	sf::Text rulesTitle(titleFont, "RULES", 32);
	rulesTitle.setOrigin({ rulesTitle.getLocalBounds().size.x / 2.f, rulesTitle.getLocalBounds().size.y / 2.f });
	rulesTitle.setPosition({ window.getSize().x / 2.f, 185.f });
	window.draw(rulesTitle);

	// Bold "Goal:"
	sf::Text goalLabel(uiFont, "Goal:\n\nHow:\n\n\nScore:", 16);
	goalLabel.setStyle(sf::Text::Bold);
	goalLabel.setPosition({ 100.f, 220.f }); // Adjust to your layout
	goalLabel.setFillColor(sf::Color::White);
	window.draw(goalLabel);

	// Rest of the sentence
	sf::Text goalText(uiFont, rulesString, 16);
	goalText.setPosition({ goalLabel.getPosition().x + goalLabel.getLocalBounds().size.x, goalLabel.getPosition().y });
	goalText.setFillColor(sf::Color::White);
	window.draw(goalText);

}

void Game::displayProjectDetails() {
	windowWidth = static_cast<float>(window.getSize().x);
	windowHeight = static_cast<float>(window.getSize().y);

	std::string storyString;
	storyString += "Puck Dropper is my first C++ project, built in just 2 weeks. It uses real-time physics-based\nmovement, custom classes, scoring, and user input.\n\n";
	storyString += "Puck Dropper is built in C++ with SFML 3.0. I relied on resources, including: Codecademy, Source\nDocumentation, ChatGPT, Google, and Stack Overflow\n\n";
	storyString += "I built this as a portfolio piece to showcase my ability to carry a project from concept to\ncompletion as well as what I can learn to build from scratch with just curiosity and coffee.\n\n";
	storyString += "Music by Twistedloop, Sounds by Kenney.nl";
	sf::Text storyText(uiFont, storyString, 16);

	sf::Text storyTitle(titleFont, "STORY", 32);

	storyTitle.setOrigin({ storyTitle.getLocalBounds().size.x / 2.f, storyTitle.getLocalBounds().size.y / 2.f });
	storyTitle.setPosition({ windowWidth / 2.f, 385.f });
	storyText.setPosition({ 100.f , storyTitle.getPosition().y + 32.5f });

	window.draw(storyTitle);
	window.draw(storyText);

}

void Game::initMusicButton() {
	musicButton.shape.setFillColor(musicButton.toggled ? Colors::Whitesmoke : Colors::LemonYellow);
	musicButton.draw(window);
}