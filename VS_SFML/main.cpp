#include <SFML/Graphics.hpp>
#include <iostream>
#include <random>
#include <vector>
#include <set>

struct Casilla {
	sf::RectangleShape casilla;
	bool obstaculo;
	sf::Vector2i pos;
	bool visitado;
	std::vector<Casilla*> vecinos;
	Casilla* predecesor;
	int h, g;
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

sf::Color colorCasillaBase(sf::Color(50, 50, 220));
sf::Color colorCasillaOscuro(sf::Color(25, 25, 110));
const int nHorizontal = 16;
const int nVertical = 16;
const float ladoCasilla = 25.f;
const float marco = 10.f;
Casilla grid[nVertical][nHorizontal];
Casilla* casillaInicio = nullptr;
Casilla* casillaDestino = nullptr;
std::set<Casilla*, CompararCasilla> casillasPorVisitar;
bool movimientoDiagonalActivo = false; // No se puede cambiar durante la ejecución del programa.
bool heuristicoActivo = true;
const int maxDistanciaManhattan = nHorizontal + nVertical - 2;

int distanciaManhattan(sf::Vector2i pos1, sf::Vector2i pos2) {
	return abs(pos2.x - pos1.x) + abs(pos2.y - pos1.y);
}

void inicializarGrid(bool reinicio = false) {
	for (int i = 0; i < nVertical; i++) {
		for (int j = 0; j < nHorizontal; j++) {
			grid[i][j].g = std::numeric_limits<int>::max();
			grid[i][j].predecesor = nullptr;
			grid[i][j].visitado = false;

			if (reinicio) continue;

			grid[i][j].obstaculo = false;
			grid[i][j].pos = sf::Vector2i(i, j);
			grid[i][j].casilla.setSize(sf::Vector2f(ladoCasilla, ladoCasilla));
			grid[i][j].casilla.setPosition(j * (ladoCasilla + 10), i * (ladoCasilla + 10));
			grid[i][j].casilla.setFillColor(colorCasillaBase);
			grid[i][j].casilla.setOutlineColor(sf::Color::White);

			if (j > 0)                grid[i][j].vecinos.push_back(&grid[i][j - 1]);  // Izquierda
			if (j < nHorizontal - 1)  grid[i][j].vecinos.push_back(&grid[i][j + 1]);  // Derecha
			if (i > 0)                grid[i][j].vecinos.push_back(&grid[i - 1][j]);  // Arriba
			if (i < nVertical - 1)    grid[i][j].vecinos.push_back(&grid[i + 1][j]);  // Abajo

			if (movimientoDiagonalActivo) {
				if (i > 0 && j > 0)								grid[i][j].vecinos.push_back(&grid[i - 1][j - 1]);
				if (i > 0 && j < nHorizontal - 1)				grid[i][j].vecinos.push_back(&grid[i - 1][j + 1]);
				if (i < nVertical - 1 && j > 0)					grid[i][j].vecinos.push_back(&grid[i + 1][j - 1]);
				if (i < nVertical - 1 && j < nHorizontal - 1)	grid[i][j].vecinos.push_back(&grid[i + 1][j + 1]);
			}
		}
	}
}

void limpiarGridAlgoritmo(bool algoritmoFinalizado_) {
	for (int i = 0; i < nVertical; ++i) {
		for (int j = 0; j < nHorizontal; ++j) {
			if (&grid[i][j] == casillaDestino) continue;
			if (&grid[i][j] == casillaInicio) continue;
			if (!grid[i][j].obstaculo) {
				grid[i][j].casilla.setFillColor(colorCasillaBase);
			}
			else {
				continue;
			}
			if (algoritmoFinalizado_) {
				if (!grid[i][j].visitado) {
					grid[i][j].casilla.setFillColor(colorCasillaOscuro);
				}
			}
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

void limpiarHeuristicos() {
	for (int i = 0; i < nVertical; ++i) {
		for (int j = 0; j < nHorizontal; ++j) {
			grid[i][j].h = 0;
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
				if (&grid[i][j] == casillaInicio || &grid[i][j] == casillaDestino) return;
				if (grid[i][j].obstaculo) {
					grid[i][j].obstaculo = false;
					grid[i][j].casilla.setFillColor(colorCasillaBase);
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
					box_->casilla.setFillColor(colorCasillaBase);
					box_->casilla.setOutlineThickness(0.f);
				}
				box_ = &grid[i][j];
				box_->casilla.setFillColor(color_);
				box_->casilla.setOutlineThickness(2.f);
				return;
			}
		}
	}
}

void colorearCasillas(Casilla* casilla_) {
	if (casilla_ == casillaInicio) return;
	float factor = 1.f - (static_cast<float>(casilla_->h) / maxDistanciaManhattan);
	int r = casillaDestino->casilla.getFillColor().r * factor;
	int g = casillaDestino->casilla.getFillColor().g * factor;
	int b = casillaDestino->casilla.getFillColor().b * factor;
	casilla_->casilla.setFillColor(sf::Color(r, g, b));
}

void mostrarCamino() {
	if (casillaDestino->predecesor == nullptr) return;
	Casilla* it = casillaDestino->predecesor;
	sf::Color colorCamino = casillaDestino->casilla.getFillColor();
	while (it->predecesor != nullptr) {
		it->casilla.setFillColor(colorCamino);
		it = it->predecesor;
	}
}

bool algoritmo_A_Estrella() {
	if (casillasPorVisitar.empty()) {
		std::cout << "No existe un camino entre ambas casillas." << std::endl;
		return false;
	}

	Casilla* n = *casillasPorVisitar.begin();  // Tomar el nodo con menor costo f = g + h
	colorearCasillas(n);
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
	const int window_width = (ladoCasilla * nHorizontal) + (marco * nHorizontal - marco);
	const int window_height = (ladoCasilla * nVertical) + (marco * nVertical - marco);
	sf::RenderWindow window(sf::VideoMode(window_width, window_height), "SFML Window");

	bool ctrlPresionado = false;
	bool altPresionado = false;
	bool algoritmoActivo = false;
	bool algoritmoFinalizado = false;
	inicializarGrid();
	const float t = 0.01f;

	std::cout << "Controles:" << std::endl;
	std::cout << "Click Izq <--- Crear/Eliminar obstaculo" << std::endl;
	std::cout << "Ctrl + Click Izq <--- Seleccionar nodoInicio" << std::endl;
	std::cout << "Alt + Click Izq <--- Seleccionar nodoFinal" << std::endl;
	std::cout << "S <--- Iniciar algoritmo" << std::endl;
	std::cout << "Tab <--- Cambiar algoritmo" << std::endl;
	std::cout << "R <--- Reiniciar variables.\n\n";
	std::cout << "Algoritmo activo: " << ((heuristicoActivo) ? "A*" : "Dijkstra") << std::endl;

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
						selecionarCasilla(mousePos, casillaInicio, sf::Color::Red);
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
					else {
						limpiarHeuristicos();
					}
					casillaInicio->g = 0;
					casillasPorVisitar.insert(casillaInicio);
					algoritmoActivo = true;
				}
				else if (event.key.code == sf::Keyboard::Tab) {
					heuristicoActivo = !heuristicoActivo;
					std::cout << "Algoritmo activo: " << ((heuristicoActivo) ? "A*" : "Dijkstra") << std::endl;
				}
				else if (event.key.code == sf::Keyboard::R) {
					inicializarGrid(true);
					casillasPorVisitar.clear();
					algoritmoFinalizado = false;
					limpiarGridAlgoritmo(algoritmoFinalizado);
				}
			}
			default:
				break;
			}
		}

		delta += clock.restart().asSeconds();
		if (delta >= t) {
			if (algoritmoActivo) {
				if (!(algoritmoActivo = algoritmo_A_Estrella())) {
					algoritmoFinalizado = true;
					limpiarGridAlgoritmo(algoritmoFinalizado);
					mostrarCamino();
				}
			}
			delta = 0.f;
		}

		window.clear(sf::Color::Black);
		mostrarGrid(window);
		window.display();
	}
}
