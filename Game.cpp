#include "Game.h"
#include <SFML/Graphics.hpp>
#include <iostream>

Game::Game()

	: window(sf::VideoMode(sf::Vector2u{ 800, 600 }), "Puck Dropper"),
	view(sf::Vector2f{ 400.f, 300.f }, sf::Vector2f{ 800.f, 600.f }),
	grid(7, 10, 60.f, 6.f, sf::Vector2f{ 800.f, 600.f })

{
	window.setView(view);
}

void Game::run() {
	while (window.isOpen()) {
		processEvents();
		update();
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
	float screenHeight = 600.f;

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

void Game::update() {
	// TODO: later animation/physics updates
}

void Game::render() {
	window.clear(sf::Color::Black);
	grid.draw(window);
	window.display();
}
