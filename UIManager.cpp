#include "UIManager.h"
#include "Button.h" 
#include <SFML/Graphics.hpp>

bool UIManager::handleClick(const sf::Vector2f& pos) {
    for (auto& button : buttons) {
        if (button.contains(pos)) {
            button.onClick();
            return true;
        }
    }
    return false;
}

void UIManager::addButton(const Button& button) {
    buttons.push_back(button);
}

void UIManager::clearButtons() {
    buttons.clear();
}

void UIManager::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    for (auto& button : buttons) {
        button.handleEvent(event, window);
    }
}

void UIManager::update(const sf::RenderWindow& window) {
    for (auto& button : buttons) {
        button.update(window);
    }
}