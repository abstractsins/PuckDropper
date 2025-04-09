#include "Game.h"
#include "UIManager.h"
#include "Colors.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>


Game::Game()
	: window(sf::VideoMode({ 800, 800 }), "Puck Dropper"),
	view({ 400.f, 300.f }, sf::Vector2f{ 800.f, 800.f }),
	grid(14, 20, 30.f, 1.5, sf::Vector2f{ 800.f, 800.f }, 0.f),
	puck(6.f, startingPos),
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
		if (currentMode != Mode::MainMenu && currentMode != Mode::About && simulationStarted) {
			puck.update(dt, grid, allowSegmentCollision, allowDotCollision, allowPuckBreak);
			if (isPuckOutOfBounds() == true) {
				puck.reset(startingPos);
			}
		}

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
						startButton.onClick();
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
	if (grid.handleClick(pos)) {
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
	if (currentMode != Mode::About && currentMode != Mode::MainMenu) {
		// For Free Form mode, update the toggle button appearance.
		if (currentMode == Mode::FreeForm) {
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

	if (currentMode == Mode::MainMenu || currentMode == Mode::About) {
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
		resetButton.draw(window);
		startButton.draw(window);
		if (currentMode == Mode::FreeForm) {
			toggleSegmentCollisionButton.draw(window);
			toggleDotCollisionButton.draw(window);
			togglePuckBreakButton.draw(window);
		}
		if (puck.broken()) {
			// Reset Prompt By
			sf::Text userResetPrompt(uiFont, "Press Any Key to Reset Puck", 24);
			userResetPrompt.setPosition({ 300.f, 35.f });
			userResetPrompt.setFillColor(Colors::Whitesmoke);
			userResetPrompt.setOutlineColor(sf::Color::Green);
			userResetPrompt.setOutlineThickness(0.25);
			window.draw(userResetPrompt);
			//userResetPrompt.draw(window);
		}
		// collision counter
		int numCollisions = puck.getCollisions();
		sf::Text collisionScore(uiFont, "Collisions: " + std::to_string(numCollisions), 18);
		collisionScore.setPosition({ 500.f, 10.f });
		collisionScore.setFillColor(Colors::Whitesmoke);
		window.draw(collisionScore);

		returnToMenuButton.draw(window);
	}

	if (currentMode == Mode::MainMenu) {
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
}

void Game::initUIButtons() {
	// Initialize simulation UI buttons.
	initResetButton();
	// (Existing toggle button for FreeForm mode gets added only if applicable.)
	if (currentMode == Mode::FreeForm) {

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

		toggleDotCollisionButton = Button(sf::Vector2(150.f, 40.f), "Dot Collision: OFF", uiFont, [this]() {
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

	startButton = Button(sf::Vector2f(100.f, 40.f), "Start", uiFont, [this]() {
		startSimulation();
		startButton.setText("Running...");
		startButton.shape.setFillColor(Colors::ForestGreen);
	});
	// Place the start button at an appropriate position (for example, next to Reset).
	startButton.setPosition(sf::Vector2f(20.f, 70.f));
	uiManager.addButton(startButton);
}

void Game::initResetButton() {
	// Create a new reset button using our Button class.
	// The callback here resets the puck when the button is clicked.
	resetButton = Button(sf::Vector2f(120.f, 40.f), "Reset Grid", uiFont, [this]() {
		gameReset();
	});
	resetButton.setPosition(sf::Vector2f(20.f, 20.f));
	resetButton.setFillColor(Colors::LightRed);
	uiManager.addButton(resetButton);
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
	currentMode = Mode::FreeForm;
	allowSegmentCollision = false;  // Default for free form; user can toggle.
	// Now initialize simulation UI (reset and toggle buttons).
	initUIButtons();
	gameReset();
}

void Game::enterScoringMode() {
	mainMenuMusic.stop();
	uiManager.clearButtons();
	currentMode = Mode::Scoring;
	allowSegmentCollision = true;  // Always collide in scoring mode.
	initUIButtons();
	gameReset();
}

void Game::enterAboutMode() {
	uiManager.clearButtons();
	currentMode = Mode::About;
	initUIButtons();
}

void Game::enterMainMenu() {
	uiManager.clearButtons();
	currentMode = Mode::MainMenu;
	initMainMenuUI();
}

void Game::gameReset(std::string resetType) {
	puck.reset(startingPos);
	simulationStarted = false;
	startButton.setText("Start");
	startButton.shape.setFillColor(Colors::LightGreen);
	if (resetType == "full") {
		grid.resetGrid();
	}
}

void Game::startSimulation() {
	simulationStarted = true;
}
