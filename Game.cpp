#include "Game.h"
#include <SFML/Graphics.hpp>
#include <iostream>

Game::Game()
	: window(sf::VideoMode(sf::Vector2u{ 800, 800 }), "Puck Dropper"),
	view(sf::Vector2f{ 400.f, 300.f }, sf::Vector2f{ 800.f, 800.f }),
	grid(7, 10, 60.f, 3.f, sf::Vector2f{ 800.f, 600.f }, 0.f),
	puck(12.f, sf::Vector2f(250.f, 0.f))  // tweak position later
{
	window.setView(view);
}

void Game::run() {
	while (window.isOpen()) {
		float dt = clock.restart().asSeconds();
		processEvents();
		update(dt);
		render();
	}
}

void Game::processEvents() {
	while (auto event = window.pollEvent()) {
		if (event->is<sf::Event::Closed>())
			window.close();

		if (event->is<sf::Event::Resized>()) {
			handleResize();
		}

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
	puck.update(dt);  // Use the delta time to update physics

	// Reset if the puck falls out of view
	const float screenBottom = 800.f;
	if (puck.getPosition().y > screenBottom + 50.f) {
		puck.reset({ 250.f, 0.f }); // or any starting position you prefer
	}
}


void Game::render() {
	window.clear(sf::Color::Black);
	grid.draw(window);
	puck.draw(window);
	window.display();
}
