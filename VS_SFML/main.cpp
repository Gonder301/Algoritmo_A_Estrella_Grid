#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <vector>
#include <set>
#include <unordered_set>

struct Casilla {
	sf::RectangleShape casilla;
	std::vector<Casilla*> vecinos;
	Casilla* predecesor;
	int h, g;
	sf::Vector2i pos;
	bool visitado = false;
	bool obstaculo = false;
};

struct CompararCasilla {
	bool operator()(const Casilla* a, const Casilla* b) const {
		int fA = a->h + a->g;
		int fB = b->h + b->g;
		if (fA != fB) {
			return fA < fB;
		}
		else {
			return a < b;
		}
	}
};

sf::Color colorBase(sf::Color(50, 50, 220));
const int nHorizontal = 15;
const int nVertical = 15;
const float lado = 25;
const float marco = 10.f;
Casilla grid[nVertical][nHorizontal];
std::set<Casilla*, CompararCasilla> casillasPorVisitar;
Casilla* casillaInicial = nullptr;
Casilla* casillaDestino = nullptr;
bool diagonalActivo = false;
bool heuristicoActivo = true;
std::vector<sf::RectangleShape*> camino;

void generarCamino() {
	Casilla* it = casillaDestino;
	sf::RectangleShape* casilla = nullptr;
	do {
		casilla = new sf::RectangleShape(sf::Vector2f(lado, lado));
		casilla->setFillColor(sf::Color::Green);
		casilla->setPosition(it->casilla.getPosition());
		camino.push_back(casilla);
		it = it->predecesor;
	} while (it != nullptr);
}

int distanciaManhattan(sf::Vector2i pos1, sf::Vector2i pos2) {
	return abs(pos2.x - pos1.x) + abs(pos2.y - pos1.y);
}

void inicializarGrid() {
	for (int i = 0; i < nVertical; i++) {
		for (int j = 0; j < nHorizontal; j++) {
			grid[i][j].pos = sf::Vector2i(i, j);
			grid[i][j].predecesor = nullptr;
			grid[i][j].casilla.setSize(sf::Vector2f(lado, lado));
			grid[i][j].casilla.setPosition(j * (lado + 10), i * (lado + 10));
			grid[i][j].casilla.setFillColor(colorBase);

			if (j > 0)                grid[i][j].vecinos.push_back(&grid[i][j - 1]);  // Izquierda
			if (j < nHorizontal - 1)  grid[i][j].vecinos.push_back(&grid[i][j + 1]);  // Derecha
			if (i > 0)                grid[i][j].vecinos.push_back(&grid[i - 1][j]);  // Arriba
			if (i < nVertical - 1)    grid[i][j].vecinos.push_back(&grid[i + 1][j]);  // Abajo

			if (diagonalActivo) {
				if (i > 0 && j > 0)								grid[i][j].vecinos.push_back(&grid[i - 1][j - 1]);
				if (i > 0 && j < nHorizontal - 1)				grid[i][j].vecinos.push_back(&grid[i - 1][j + 1]);
				if (i < nVertical - 1 && j > 0)					grid[i][j].vecinos.push_back(&grid[i + 1][j - 1]);
				if (i < nVertical - 1 && j < nHorizontal - 1)	grid[i][j].vecinos.push_back(&grid[i + 1][j + 1]);
			}
			grid[i][j].g = std::numeric_limits<int>::max();
		}
	}
}

void cambiarColorBase() {
	for (int i = 0; i < nVertical; ++i) {
		for (int j = 0; j < nHorizontal; ++j) {
			if (!grid[i][j].obstaculo) grid[i][j].casilla.setFillColor(colorBase);
		}
	}
}

void inicializarHeuristicos() {
	for (int i = 0; i < nVertical; ++i) {
		for (int j = 0; j < nHorizontal; ++j) {
			grid[i][j].h = distanciaManhattan(grid[i][j].pos, casillaDestino->pos);
		}
	}
}

void mostrarGrid(sf::RenderWindow& window) {
	for (int i = 0; i < nVertical; ++i) {
		for (int j = 0; j < nHorizontal; ++j) {
			window.draw(grid[i][j].casilla);
		}
	}
}

void crearObstaculo(sf::Vector2f mousePos) {
	for (int i = 0; i < nVertical; ++i) {
		for (int j = 0; j < nHorizontal; ++j) {
			if (grid[i][j].casilla.getGlobalBounds().contains(mousePos)) {
				if (grid[i][j].obstaculo) {
					grid[i][j].obstaculo = false;
					grid[i][j].casilla.setFillColor(colorBase);
				}
				else {

					grid[i][j].obstaculo = true;
					grid[i][j].casilla.setFillColor(sf::Color(127, 127, 127));
				}
				return;
			}
		}
	}
}

void selecionarCasilla(sf::Vector2f mousePos, Casilla*& box_, sf::Color color_) {
	for (int i = 0; i < nVertical; ++i) {
		for (int j = 0; j < nHorizontal; ++j) {
			if (grid[i][j].casilla.getGlobalBounds().contains(mousePos)) {
				if (box_) {
					box_->casilla.setFillColor(colorBase);
				}
				grid[i][j].casilla.setFillColor(color_);
				box_ = &grid[i][j];
				return;
			}
		}
	}
}

bool algoritm_A_Estrella() {
	if (casillasPorVisitar.empty()) {
		std::cout << "No se encontró ningún camino entre las casillas." << std::endl;
		return false;
	}

	Casilla* n = *casillasPorVisitar.begin();  // Tomar el nodo con menor costo f = g + h
	n->casilla.setFillColor(sf::Color::Magenta);
	casillasPorVisitar.erase(casillasPorVisitar.begin());  // Eliminarlo del conjunto

	int temp_g = 0;
	for (auto& it : n->vecinos) {
		if (it->visitado) continue;  // Si ya fue visitado, ignorarlo
		if (it->obstaculo) continue; // Si es un obstaculo, ignorarlo

		temp_g = n->g + 1; 
		if (temp_g < it->g) {
			// Se actualiza solo si encontramos un mejor camino
			casillasPorVisitar.erase(it);  // IMPORTANTE: eliminarlo antes de modificar `g`
			it->g = temp_g;
			it->predecesor = n;
			casillasPorVisitar.insert(it);  // Reinsertar con el nuevo costo
		}
	}
	n->visitado = true;

	if (n != casillaDestino) {
		return true;
	}
	else {
		std::cout << "Algoritmo finalizado." << std::endl;
		return false;
	}
}

int main()
{
	const int window_width = (lado * nHorizontal) + (marco * nHorizontal - marco);
	const int window_height = (lado * nVertical) + (marco * nVertical - marco);
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "SFML Window");

	bool ctrlPresionado = false;
	bool altPresionado = false;
	bool algoritmoActivo = false;
	bool algoritmoFinalizado = false;
	inicializarGrid();
	const float t = 0.01f;

	float delta = 0.f;
	sf::Clock clock;
	while (window.isOpen())
	{
		sf::Event event;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
			ctrlPresionado = true;
		}
		else {
			ctrlPresionado = false;
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt)) {
			altPresionado = true;
		}
		else {
			altPresionado = false;
		}

		while (window.pollEvent(event))
		{
			switch (event.type) {
			case sf::Event::Closed:
			{
				window.close();
				break;
			}
			case sf::Event::MouseButtonPressed:
			{
				sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

				if (event.mouseButton.button == sf::Mouse::Left) {
					if (ctrlPresionado) {
						selecionarCasilla(mousePos, casillaInicial, sf::Color::Red);
					}
					else if (altPresionado) {
						selecionarCasilla(mousePos, casillaDestino, sf::Color::Green);
					}
					else {
						crearObstaculo(mousePos);
					}
				}
			}
			case sf::Event::KeyReleased:
			{
				if (event.key.code == sf::Keyboard::S) {
					if (heuristicoActivo) {
						inicializarHeuristicos();
					}
					casillaInicial->g = 0;
					casillasPorVisitar.insert(casillaInicial);
					algoritmoActivo = true;
				}
			}
			default:
				break;
			}
		}

		delta += clock.restart().asSeconds();
		if (delta >= t) {
			if (algoritmoActivo) {
				if (!(algoritmoActivo = algoritm_A_Estrella())) {
					std::cout << "fin" << std::endl;
					cambiarColorBase();
					generarCamino();
					algoritmoFinalizado = true;
				}
			}
			delta = 0.f;
		}

		window.clear(sf::Color::Black);
		mostrarGrid(window);

		if (algoritmoFinalizado) {
			for (auto& it : camino) {
				window.draw(*it);
			}
		}
		window.display();
	}
}