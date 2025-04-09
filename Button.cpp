#include "Button.h"
#include "Colors.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <iostream>

sf::Font Button::defaultFont;

const sf::Font& Button::getDefaultFont() {
    static bool loaded = false;
    if (!loaded) {
        if (!defaultFont.openFromFile("Tuffy.ttf")) {
            throw std::runtime_error("Failed to load default font");
        }
        loaded = true;
    }
    return defaultFont;
}

Button::Button()
    : callback(nullptr),
    text(getDefaultFont(), "", 16)
{
    shape.setSize({ 0.f, 0.f });
    //shape.setFillColor(backgroundColorNormal);
}

Button::Button(const sf::Vector2f& size,
    const std::string& label,
    const sf::Font& font,
    std::function<void()> callback)
    : callback(callback),
    text(font, label, 16)
{
    shape.setSize(size);
    //shape.setFillColor(backgroundColorNormal);
    text.setFillColor(sf::Color::Black);
    centerText();
}

void Button::setPosition(const sf::Vector2f& position) {
    shape.setPosition(position);
    centerText();
}

void Button::centerText() {
    sf::FloatRect bounds = text.getLocalBounds();
    sf::Vector2f size = bounds.size; 
    text.setOrigin(size / 2.f);
    text.setPosition(shape.getPosition() + shape.getSize() / 2.f);
}

void Button::draw(sf::RenderTarget& target) const {
    target.draw(shape);
    target.draw(text);
}

void Button::onClick() const {
    if (callback) {
        std::cout << "Button: '" << text.getString().toAnsiString() << "' clicked\n";
        callback();
    }
}

bool Button::contains(const sf::Vector2f& point) const {
    return shape.getGlobalBounds().contains(point);
}

void Button::setFillColor(sf::Color color) {
    shape.setFillColor(color);
}

void Button::setText(const std::string& newLabel) {
    text.setString(newLabel);
    centerText();
}

//void Button::update(const sf::RenderWindow& window) {
//    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
//
//    //std::cout << "UPDATE CALLED: '" << text.getString().toAnsiString() << "'\n\n";
//
//    // For non-toggle buttons, "active" should only be true while mouse is down.
//    if (contains(mousePos)) {
//        std::cout << "'" << text.getString().toAnsiString() << "' Contains mousePOS\n\n";
//        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
//
//            // Set active color.
//            setFillColor(backgroundColorActive);  // e.g., a color you define
//            active = true;
//        }
//        else {
//            // Set hover color (you might have backgroundColorHover).
//            setFillColor(backgroundColorHover);
//            active = false;
//        }
//    }
//    else {
//        active = false;
//        // When not hovered, display normal color.
//        shape.setFillColor(backgroundColorNormal);
//        shape.setFillColor(sf::Color::Black);
//    }
//}

void Button::update(const sf::RenderWindow& window) {
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (contains(mousePos)) {
        std::cout << "'" << text.getString().toAnsiString() << "' Contains mousePOS\n\n";

        setFillColor(sf::Color::Black);

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
            std::cout << "'" << text.getString().toAnsiString() << "' Clicked\n\n";


        }
        else {

        }
    }
    else {

    }
}


void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window)
{
    if (event.is<sf::Event::MouseButtonPressed>()) {
        if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mousePress->button == sf::Mouse::Button::Left) {
                sf::Vector2f mousePos = window.mapPixelToCoords({ mousePress->position.x, mousePress->position.y });
                if (contains(mousePos)) {
                    active = true;
                    // Optionally set the color immediately.
                    //shape.setFillColor(backgroundColorActive);
                }
            }
        }
    }
    else if (event.is<sf::Event::MouseButtonReleased>()) {
        if (const auto* mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseRelease->button == sf::Mouse::Button::Left) {
                // On release, reset active state.
                active = false;
            }
        }
    }
}


//if (const auto* mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
//    if (mouseRelease->button == sf::Mouse::Button::Left) {
//        sf::Vector2f mousePos = window.mapPixelToCoords({ mouseRelease->position.x, mouseRelease->position.y });
//        if (active && contains(mousePos)) {
//            // On release, trigger the callback.
//            onClick();
//        }
//        // Reset active state regardless.
//        active = false;
//    }
//}