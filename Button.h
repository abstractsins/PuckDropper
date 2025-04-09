#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <iostream>

class Button {
public:
    Button();

    Button(
        const sf::Vector2f& size,
        const std::string& label,
        const sf::Font& font = Button::getDefaultFont(),
        std::function<void()> callback = nullptr
    );

    void setPosition(const sf::Vector2f& position);
    void draw(sf::RenderTarget& target) const;
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    
    void setText(const std::string& newLabel);
    void setFillColor(sf::Color color);
    void update(const sf::RenderWindow& window);

    //void handleEvent();
    void onClick() const;
    bool contains(const sf::Vector2f& point) const;

    static const sf::Font& getDefaultFont();

    bool toggled;
    sf::RectangleShape shape;

private:
    bool active;

    sf::Text text;

    std::function<void()> callback;

    static sf::Font defaultFont;

    void centerText();
};

#endif
