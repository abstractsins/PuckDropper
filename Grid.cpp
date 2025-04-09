#include "Grid.h"
#include "Colors.h"

Grid::Grid(int cols, int rows, float spacing, float dotRadius, sf::Vector2f screenSize, float padding)
	: cols(cols), rows(rows), spacing(spacing), dotRadius(dotRadius)
{
	float extraOffset = 100.f;

	float usableWidth = screenSize.x - 2 * padding;
	float usableHeight = screenSize.y - 2 * padding;

	origin.x = padding + extraOffset + (usableWidth - (cols - 1) * spacing) / 2.f;
	origin.y = padding + (usableHeight - (rows - 1) * spacing) / 2.f;

	sf::Vector2f size{
		(cols - 1) * spacing,
		(rows - 1) * spacing
	};

	sf::Vector2f boundsPos = origin - sf::Vector2f(15.f, 15.f);
	sf::Vector2f boundsSize = size + sf::Vector2f(30.f, 30.f); // buffer padding

	setBounds(boundsPos, boundsSize);


	// LEFT and RIGHT WALLS
	for (int y = 0; y < rows - 1; ++y) {
		// left side column 0
		connections.push_back({ {0, y}, {0, y + 1}, false, Colors::DarkPurple });

		// right side last column 
		int lastCol = cols - 1;
		connections.push_back({ {lastCol, y}, {lastCol, y + 1}, false, Colors::DarkPurple });
	}
}

void Grid::setBounds(const sf::Vector2f& position, const sf::Vector2f& size) {
	gridBounds = sf::FloatRect(position, size);
}

sf::Vector2f Grid::getDotPosition(int col, int row) const {
	return {
		origin.x + col * spacing,
		origin.y + row * spacing
	};
}

void Grid::draw(sf::RenderWindow& window) {
	sf::CircleShape dot(dotRadius);
	dot.setOrigin(sf::Vector2f{ dotRadius, dotRadius });
	dot.setFillColor(sf::Color::Cyan);

	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			dot.setPosition(getDotPosition(col, row));
			window.draw(dot);
		}
	}

	for (const auto& conn : connections) {
		sf::Vector2f start = getDotPosition(conn.from.x, conn.from.y);
		sf::Vector2f end = getDotPosition(conn.to.x, conn.to.y);

		sf::Vector2f direction = end - start;
		float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
		float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159f;

		sf::RectangleShape line(sf::Vector2f(length, 4.f)); // 4.f is thickness
		line.setFillColor(conn.color);
		line.setOrigin(sf::Vector2f(0.f, 2.f)); // center it vertically
		line.setPosition(start);
		line.setRotation(sf::degrees(angle));

		window.draw(line);
	}

	if (selectedDot.has_value()) {
		sf::CircleShape highlight(dotRadius + 4.f);
		highlight.setFillColor(sf::Color::Transparent);
		highlight.setOutlineColor(sf::Color::Yellow);
		highlight.setOutlineThickness(2.f);
		highlight.setOrigin(sf::Vector2f(dotRadius + 4.f, dotRadius + 4.f));
		highlight.setPosition(getDotPosition(selectedDot->x, selectedDot->y));
		window.draw(highlight);
	}

}

void Grid::addConnection(int col1, int row1, int col2, int row2) {
	connections.push_back({ {col1, row1}, {col2, row2} });
}

bool Grid::handleClick(sf::Vector2f mousePos) {

	if (!gridBounds.contains(mousePos)) return false;

	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			sf::Vector2f pos = getDotPosition(col, row);
			float distance = std::hypot(mousePos.x - pos.x, mousePos.y - pos.y);
				
			if (distance <= dotRadius + 15.f) {
				sf::Vector2i clicked(col, row);

				if (selectedDot.has_value()) {
					sf::Vector2i delta = clicked - selectedDot.value();

					int dx = std::abs(delta.x);
					int dy = std::abs(delta.y);
					bool isNeighbor = (dx <= 1 && dy <= 1) && (dx + dy > 0);

					if (isNeighbor) {
						if (const Connection* conn = getConnection(selectedDot.value(), clicked)) {
							if (conn->isEditable) {
								removeConnection(selectedDot.value(), clicked);
							}
						}
						else {
							addConnection(
								selectedDot.value().x, selectedDot.value().y,
								clicked.x, clicked.y
							);
						}
					}

					selectedDot.reset();  // always deselect after second click
				}
				else {
					selectedDot = clicked;  // first selection
				}

				return true;  // done, don't continue checking
			}
		}
	}


	// If nothing was close enough, deselect
	selectedDot.reset();
	return true;
}

const Grid::Connection* Grid::getConnection(sf::Vector2i from, sf::Vector2i to) const {
	for (const auto& conn : connections) {
		if ((conn.from == from && conn.to == to) || (conn.from == to && conn.to == from)) {
			return &conn;
		}
	}
	return nullptr;
}


void Grid::removeConnection(sf::Vector2i from, sf::Vector2i to) {
	connections.erase(std::remove_if(connections.begin(), connections.end(),
		[&](const Connection& conn) {
			return (conn.from == from && conn.to == to) || (conn.from == to && conn.to == from);
		}), connections.end());
}

const std::vector<Grid::Connection>& Grid::getConnections() const {
	return connections;
}

void Grid::resetGrid() {
	// Clear any user-added or modified connections.
	connections.clear();

	// Re-create the original left and right wall connections as defined in the constructor.
	for (int y = 0; y < rows - 1; ++y) {
		// Left wall: connection from (0, y) to (0, y + 1)
		connections.push_back({ {0, y}, {0, y + 1}, false, Colors::DarkPurple });

		// Right wall: connection from (cols-1, y) to (cols-1, y + 1)
		int lastCol = cols - 1;
		connections.push_back({ {lastCol, y}, {lastCol, y + 1}, false, Colors::DarkPurple });
	}

	// Optionally, clear any selection.
	selectedDot.reset();
}