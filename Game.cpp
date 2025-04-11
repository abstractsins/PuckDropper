#include "Game.h"
#include "UIManager.h"
#include "Colors.h"
#include "Mode.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>


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
	while (window.isOpen()) {
		float dt = clock.restart().asSeconds();
		processEvents();

		// Only update the puck if the simulation has started.
		if (currentMode != Mode::Main && currentMode != Mode::About && simulationStarted) {
			puck.update(dt, grid, allowSegmentCollision, allowDotCollision, allowPuckBreak, puckLanded);
			if (isPuckOutOfBounds() == true) {
				puck.reset(startingPos);
				runClock.restart();
			}
		}

		if (currentMode == Mode::Scoring) {
			if (!puckLanded) {
				sf::Vector2f puckPos = puck.getPosition();
				sf::Vector2f velocity = puck.getVelocity(); // assume you have this
				//std::cout << std::to_string(velocity.x) << " , " << std::to_string(velocity.y) << '\n';

				// Check if it's in the bucket region (y-position wise)
				if (puckPos.y > grid.getDotPosition(0, grid.getRows() - 3).y) {
					// Check if it's basically stopped
					if (std::abs(velocity.x) < 3 && std::abs(velocity.y) < 2) {
						puckLanded = true;
						puck.setFillColor(sf::Color::Green);
						elapsedTime = runClock.getElapsedTime().asSeconds();

						int col = grid.getSlotIndexFromX(puckPos.x); // you may need to write this helper
						std::cout << "Puck landed in column: " << col << "\n";

						score = grid.scoreValues[col];

						scoreCalculator(score);

					}
				}
			}
		}

		if (puck.broken()) { runClock.stop(); }

		render();
	}
}


void Game::processEvents() {
	uiManager.update(window);

	while (auto event = window.pollEvent()) {

		if (event->is<sf::Event::Closed>())
			window.close();

		if (event->is<sf::Event::Resized>()) {
			handleResize();
		}

		if (event->is<sf::Event::KeyPressed>()) {
			if (puck.broken()) {
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
	//else if (currentMode == Mode::Scoring && running) {
		// Don’t allow editing during scoring simulation
		//return; // off for debug
	//} 
	else if (grid.handleClick(pos)) {
		std::cout << "grid called\n";
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
		// Title
		sf::Text titleText(titleFont, "Puck Dropper", 86);
		titleText.setPosition({ 100.f, 50.f });
		titleText.setFillColor(sf::Color::Yellow);
		titleText.setOutlineColor(sf::Color::Black); 
		titleText.setOutlineThickness(0.5); 
		window.draw(titleText);

		// Created By
		sf::Text createdByText(uiFont, "Created by Daniel Berlin, 2025", 16);
		createdByText.setPosition({ 550.f, 700.f });
		createdByText.setFillColor(Colors::Whitesmoke);
		createdByText.setOutlineColor(sf::Color::Black);
		createdByText.setOutlineThickness(0.5);
		window.draw(createdByText);
	}
	else {
		// Simulation mode drawing.
		grid.draw(window);
		puck.draw(window);
		resetGridButton.draw(window);
		resetPuckButton.draw(window);
		startButton.draw(window);
		if (currentMode == Mode::Free) {
			toggleSegmentCollisionButton.draw(window);
			toggleDotCollisionButton.draw(window);
			togglePuckBreakButton.draw(window);
		}
		if (puck.broken()) {
			// Reset Prompt By
			sf::Text userResetPrompt(uiFont, "Press Any Key to Reset Puck", 24);
			userResetPrompt.setPosition({ 300.f, 50.f });
			userResetPrompt.setFillColor(Colors::Whitesmoke);
			userResetPrompt.setOutlineColor(sf::Color::Green);
			userResetPrompt.setOutlineThickness(0.25);
			window.draw(userResetPrompt);
			//userResetPrompt.draw(window);
		}

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

		returnToMenuButton.draw(window);

		if (puckLanded) {
			sf::RectangleShape scoreReadoutArea({ 225.f, 175.f });

			scoreReadoutArea.setPosition({ 20.f, 580.f });
			scoreReadoutArea.setFillColor(sf::Color::Transparent);
			scoreReadoutArea.setOutlineColor(sf::Color::Cyan);
			scoreReadoutArea.setOutlineThickness(1.f);

			std::string scoreReadout = scoreOutputText(score, timeBonusMultiplier, puck.getCollisions(), grid.getNumConnections(), totalScore);
			sf::Text scoreReadoutText(uiFont, scoreReadout, 16);
			std::cout << scoreReadoutText.getString().toAnsiString() << "\n";
			float padding = 20.f;
			scoreReadoutText.setPosition({ scoreReadoutArea.getPosition().x + padding, scoreReadoutArea.getPosition().y + padding });

			window.draw(scoreReadoutArea);
			window.draw(scoreReadoutText);
		}
	}

	if (currentMode == Mode::Main) {
		// Then draw the main menu buttons.
		window.draw(menuContainer);
		freeModeButton.draw(window);
		scoringModeButton.draw(window);
		rulesModeButton.draw(window);
		musicButton.shape.setFillColor(musicButton.toggled ? Colors::Whitesmoke : Colors::LemonYellow);
		musicButton.draw(window);
		musicButton.setFillColor(musicButton.hover ? sf::Color::Cyan : sf::Color::Black);

	} 
	else if (currentMode == Mode::About) {
		returnToMenuButton.draw(window);
		// display rules
	}

	window.display();
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
	returnToMenuButton.setPosition(sf::Vector2f(20.f, 520.f));
	uiManager.addButton(returnToMenuButton);
}

//
// Main Menu UI initialization:
//
void Game::initMainMenuUI() {
	mainMenuMusic.setLooping(true);
	mainMenuMusic.play();

	// Clear existing buttons from the UI.
	uiManager.clearButtons();

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
	float windowWidth = static_cast<float>(window.getSize().x);
	float windowHeight = static_cast<float>(window.getSize().y);
	//menuContainer.setPosition({ windowWidth / 2.f, windowHeight / 2.f });
	menuContainer.setPosition({ 400.f, 400.f });

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
	musicButton.setPosition(sf::Vector2f(575.f, 600.f));
	uiManager.addButton(musicButton);
}



//
// Mode switching:
//
void Game::enterFreeFormMode() {
	mainMenuMusic.stop();
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

void Game::scoreCalculator(int score) {
	totalScore += score;
	int time = runClock.getElapsedTime().asMilliseconds();
	timeBonusMultiplier = timeBonusCalculator(time);
	totalScore *= timeBonusMultiplier;
	totalScore -= grid.getNumConnections();
	totalScore -= puck.getCollisions() * 4;
	std::cout << "time: " << std::to_string(time) << "ms\n\n";
	std::cout << "timeBonusPoints: " << std::to_string(timeBonusMultiplier) << "\n\n";
	std::cout << std::to_string(totalScore) << "\n\n";
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