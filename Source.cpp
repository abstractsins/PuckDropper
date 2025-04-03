#include <iostream>
#include <SFML/Graphics.hpp>

int main() {
	sf::Vector2u size{ 800, 600 };
	sf::RenderWindow window(sf::VideoMode{ size, 32 }, "Puck Dropper");

	sf::View view;
	view.setCenter(sf::Vector2f{ 400.f, 300.f });
	view.setSize(sf::Vector2f{ 800.f, 600.f });
	window.setView(view);


	const int cols = 7;
	const int rows = 10;
	const float dotRadius = 6.f;
	const float spacing = 60.f;

	sf::Vector2f gridOrigin{
		(800 - (cols - 1) * spacing) / 2.f,
		(600 - (rows - 1) * spacing) / 2.f
	};


	while (window.isOpen()) {
		while (auto event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				window.close();
			}

			if (event->is<sf::Event::Resized>()) {
				std::cout << "RESIZE\n";
				// set screen size
				float screenWidth = 800.f;
				float screenHeight = 600.f;

				// get the resized size
				sf::Vector2u size = window.getSize();

				// setup my wanted aspect ratio
				float  heightRatio = screenHeight / screenWidth;
				float  widthRatio = screenWidth / screenHeight;

				// adapt the resized window to my wanted aspect ratio
				if (size.y * widthRatio <= size.x)
				{
					size.x = size.y * widthRatio;
				}
				else if (size.x * heightRatio <= size.y)
				{
					size.y = size.x * heightRatio;
				}
				// set the new size
				window.setSize(size);
			}

		}

		window.clear(sf::Color::Black);

		for (int row = 0; row < rows; ++row) {
			for (int col = 0; col < cols; ++col) {
				sf::CircleShape dot(dotRadius);
				dot.setFillColor(sf::Color::Cyan);

				float x = gridOrigin.x + col * spacing - dotRadius;
				float y = gridOrigin.y + row * spacing - dotRadius;
				dot.setPosition({ x, y });

				window.draw(dot);
			}
		}

		window.display();
	}

	return 0;
}
