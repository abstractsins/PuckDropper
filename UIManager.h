#pragma once
#include "Button.h" 
#include <SFML/Graphics.hpp>

class UIManager {

public: 
	bool handleClick(const sf::Vector2f& pos);
	void addButton(const Button& button);
	void clearButtons();
	void update(const sf::RenderWindow& window);
	void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

private:
	std::vector<Button> buttons;
};