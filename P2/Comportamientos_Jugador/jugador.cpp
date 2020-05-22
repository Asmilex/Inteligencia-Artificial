#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>
#include <unordered_set>


double distancia (int fila_0, int columna_0, int fila_destino, int columna_destino) {
	return abs(fila_0 - fila_destino) + abs(columna_0 - columna_destino);
};


// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	static int recalculaciones = 0;

	if (sensores.vida%4 == 0) {
		acabo_de_calcular = false;
	}

	if (sensores.nivel == 4) {
		actualizar_mapaResultado(sensores);

		if (cuenta_nuevos_descub > 15) {
			recalcular_nuevos_descub = true;
			cuenta_nuevos_descub = 0;
		}
	}


	actual.fila        = sensores.posF;
	actual.columna     = sensores.posC;
	actual.orientacion = sensores.sentido;
	fil                = actual.fila;
	col                = actual.columna;
	brujula            = actual.orientacion;

	if (destino.fila == actual.fila && destino.columna == actual.columna) {
		hayPlan           = false;
		recalculaciones   = 0;
		acabo_de_calcular = false;
		powerup_avistado  = false;
	}

	if (!hayPlan) {
		destino.fila    = sensores.destinoF;
		destino.columna = sensores.destinoC;
	}


	if (mapaResultado[actual.fila][actual.columna] == 'X') {
		powerup_avistado = false;
	}
	if (mapaResultado[actual.fila][actual.columna] == 'D') {
		zapatillas = true;
		if (!zapatillas)
			powerup_avistado = false;
	}
	if (mapaResultado[actual.fila][actual.columna] == 'K') {
		bikini = true;
		if (!bikini)
			powerup_avistado = false;
	}


	// No se hacen funciones con efectos secundarios
	estado copia_actual = actual;

	if (	plan.empty()
		||  (plan.size() > 0 && plan.front() == actFORWARD && HayObstaculoDelante(copia_actual))	// He chocado con algo y no lo tenía previsto.
		||  hay_NPC_delante(sensores)
		||  (recalcular_nuevos_descub && distancia(actual.fila, actual.columna, destino.fila, destino.columna) > 10 && !powerup_avistado) ) {

		hayPlan = pathFinding(sensores, actual, destino, plan);
		recalculaciones++;


		if (recalcular_nuevos_descub)
			recalcular_nuevos_descub = false;
	}



	if (sensores.nivel == 4) {
		pair<int, int> powerups_cercanos = hay_powerups_cerca(sensores);

		if (powerups_cercanos.first != -1 && powerups_cercanos.second != -1) {
			powerup_avistado = true;


			destino.fila = powerups_cercanos.first;
			destino.columna = powerups_cercanos.second;

			bool es_recarga = mapaResultado[powerups_cercanos.first][powerups_cercanos.second] == 'X';

			if (!acabo_de_calcular) {
				if (!es_recarga || es_recarga && sensores.bateria < limite_inf_bateria)
				hayPlan = pathFinding(sensores, actual, destino, plan);
				acabo_de_calcular = true;
			}
		}
	}

	Action siguiente_accion = actIDLE;

	bool estoy_recargando = sensores.nivel == 4 && mapaResultado[actual.fila][actual.columna] == 'X' && sensores.bateria < min(int(sensores.vida * 1.5), limite_inf_bateria);


	if ((hayPlan && plan.size() > 0) && (!estoy_recargando)) {
		siguiente_accion = plan.front();
		plan.erase(plan.begin());
	}

  	return siguiente_accion;
}

//
// ────────────────────────────────────────────────────────────── PATHFINDING ─────
//

// Llama al algoritmo de busqueda que se usará en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan){
	switch (sensor.nivel){
		case 1:
			cout << "Busqueda en profundad\n";
			return pathFinding_Profundidad(origen,destino,plan);
			break;

		case 2:
			cout << "Busqueda en Anchura\n";
			return pathFinding_Anchura(origen, destino, plan);
			break;

		case 3:
			cout << "Busqueda Costo Uniforme\n";
			int bateria;
			return pathFinding_Costo_Uniforme(sensor, origen, destino, plan);
			break;

		case 4:
			cout << "Busqueda para el reto\n";
			return A_estrella(origen, destino, plan, sensor);
			break;
	}

	cout << "Comportamiento sin implementar\n";

	return false;
}


//
// ──────────────────────────────────────────────────────── UTILIDADES VARIAS ─────
//


// Dado el código en carácter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' || casilla=='M')
		return true;
	else
	  return false;
}


// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 || fil>=mapaResultado.size())
		return true;
	if (col<0 || col>=mapaResultado[0].size())
		return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parámetro st poniendo la casilla de delante.
    	st.fila    = fil;
    	st.columna = col;
		return false;
	}
	else {
		return true;
	}
}

bool ComportamientoJugador::hay_NPC_delante(Sensores sensor) {
	return sensor.superficie[2] == 'a';
}




//
// ────────────────────────────────────────────────────────────── I ──────────
//   :::::: P R O F U N D I D A D : :  :   :    :     :        :          :
// ────────────────────────────────────────────────────────────────────────
//



struct nodo{
	estado st;
	list<Action> secuencia;
};

struct ComparaEstados{
	bool operator()(const estado &a, const estado &n) const{
		if ((a.fila > n.fila) || (a.fila == n.fila && a.columna > n.columna) ||
	      (a.fila == n.fila && a.columna == n.columna && a.orientacion > n.orientacion))
			return true;
		else
			return false;
	}
};


// Implementación de la búsqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> generados;   // Lista de Cerrados
	stack<nodo> pila;						// Lista de Abiertos

	nodo current;
	current.st = origen;
	current.secuencia.clear();

	pila.push(current);

  	while (!pila.empty() && (current.st.fila != destino.fila || current.st.columna != destino.columna)){
		pila.pop();
		generados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (generados.find(hijoTurnR.st) == generados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			pila.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (generados.find(hijoTurnL.st) == generados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			pila.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (generados.find(hijoForward.st) == generados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				pila.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la pila
		if (!pila.empty()){
			current = pila.top();
		}
	}

  	cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila && current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

//
// ────────────────────────────────────────────────────── II ──────────
//   :::::: A N C H U R A : :  :   :    :     :        :          :
// ────────────────────────────────────────────────────────────────
//


bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	cout << "Calculando plan\n";
	plan.clear();

	set<estado,ComparaEstados> generados;
	queue<nodo> a_expandir;

	nodo actual;
	actual.st = origen;
	actual.secuencia.clear();

	a_expandir.push(actual);

	while (!a_expandir.empty() && (actual.st.fila != destino.fila || actual.st.columna != destino.columna)) {
		a_expandir.pop();
		generados.insert(actual.st);

		nodo hijo_TR = actual;
		hijo_TR.st.orientacion = (hijo_TR.st.orientacion + 1)%4;
		if (generados.find(hijo_TR.st) == generados.end()) { // Si no está el hijo en la lista de generados
			hijo_TR.secuencia.push_back(actTURN_R);
			a_expandir.push(hijo_TR);
		}

		nodo hijo_TL = actual;
		hijo_TL.st.orientacion = (hijo_TL.st.orientacion + 3)%4;
		if (generados.find(hijo_TL.st) == generados.end()) { // Si no está el hijo en la lista de generados
			hijo_TL.secuencia.push_back(actTURN_L);
			a_expandir.push(hijo_TL);
		}

		nodo hijo_forward = actual;
		if (!HayObstaculoDelante(hijo_forward.st)) {
			if (generados.find(hijo_forward.st) == generados.end()) {
				hijo_forward.secuencia.push_back(actFORWARD);
				a_expandir.push(hijo_forward);
			}
		}

		if (!a_expandir.empty()) {
			actual = a_expandir.front();
			//cout << "Nodo a analizar:" << actual.st.fila << ", " << actual.st.columna << endl;
		}
	}

	cout << "Búsqueda terminada\n";

	if (actual.st.fila == destino.fila && actual.st.columna == destino.columna) {
		cout << "Cargando plan\n";
		plan = actual.secuencia;

		cout << "Longitud del plan:" << plan.size() << endl;
		PintaPlan(plan);

		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "F en el chat, no hemos encontrado plan\n";
		return false;
	}
}


//
// ──────────────────────────────────────────────────────────────────── III ────────
//   :::::: C O S T O   U N I F O R M E : :  :   :    :     :        :          :
// ──────────────────────────────────────────────────────────────────────────────
//
//
// ───────────────── ESTRUCTURAS DE DATOS NECESARIAS PARA UNIFORM COST SEARCH ─────
//

	// Extensión de los nodos para contabilizar el gasto de energía.
	struct nodo_bateria {
		nodo node;
		int bateria_restante;
		bool zapatillas;
		bool bikini;
	};



	// Extensión del estado para contabilizar la batería.
	struct estado_bateria {
		estado st;
		int bateria;
		bool zapatillas;
		bool bikini;

		bool operator==(const estado_bateria& otro) const {
			return 		st.fila == otro.st.fila
					&& 	st.columna == otro.st.columna
					&&  st.orientacion == otro.st.orientacion
					&& 	bateria == otro.bateria
					&& 	zapatillas == otro.zapatillas
					&& 	bikini == otro.bikini;
		}

		bool operator< (const estado_bateria& otro) const {
			if ((st.fila > otro.st.fila) || (st.fila == otro.st.fila && st.columna > otro.st.columna) ||
		      (st.fila == otro.st.fila && st.columna == otro.st.columna && st.orientacion > otro.st.orientacion))
				return true;
			else
				return false;
		}
	};

	// Estructura:
	// Bateria -> B, fila -> f, columna -> c, zapatilla -> z, bikini -> b
	// ffccBBBBzb
	struct hash_estado_bateria {
		size_t operator()(const estado_bateria& state) const {
			return 		state.st.fila * 100000000 + state.st.columna * 1000000
					+ 	state.bateria * 100
					+   state.zapatillas * 10 + state.bikini;
		}
	};


//
// ───────────────────────────────────────────────────── FUNCIONES AUXILIARES ─────
//


int ComportamientoJugador::calcular_costo_bateria(estado state, Action accion, bool zapatillas, bool bikini) {
	int sig_fila = state.fila, sig_col = state.columna;

	if (accion = actFORWARD) {
		switch (state.orientacion) {
			case 0: sig_fila--; break;
			case 1: sig_col++; break;
			case 2: sig_fila++; break;
			case 3: sig_col--; break;
		}
	}

	int costo;
	char casilla = mapaResultado[sig_fila][sig_col];

	switch (casilla)
	{
		case 'A':
			if (bikini)
				costo = 10;
			else
				costo = 100;

			break;

		case 'B':
			if (zapatillas)
				costo = 5;
			else
				costo = 50;

			break;

		case 'T':
			costo = 2;
			break;

		case '?':
			if (bikini && zapatillas)
				costo =  7;
			else if (bikini)
				costo = 10;
			else if (zapatillas)
				costo = 5;
			break;

		case 'X':
			costo = -10;
			break;

		default:
			costo = 1;
			break;
	}

	return costo;
}

//
// ─────────────────────────────────────────────────────────── COSTO UNIFORME ─────
//

bool ComportamientoJugador::pathFinding_Costo_Uniforme(const Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan) {
	cout << "Calculando ruta\n";
	plan.clear();



//
// ─────────────────────────────────────────────── ESTRUCTURAS DE DATOS CLAVE ─────
//

	auto prioridad_nodo_bateria = [&destino](const nodo_bateria& n1, const nodo_bateria& n2) {
		return n1.bateria_restante - distancia(n1.node.st.fila, n1.node.st.columna, destino.fila, destino.columna)
					<
			   n2.bateria_restante - distancia(n2.node.st.fila, n2.node.st.columna, destino.fila, destino.columna) ;
	};

	priority_queue<nodo_bateria, vector<nodo_bateria>, decltype(prioridad_nodo_bateria)> a_expandir(prioridad_nodo_bateria);

	unordered_set<estado_bateria, hash_estado_bateria> generados;
	nodo_bateria mejor_solucion = {};

//
// ─────────────────────────────────────────────── OPTIMIZACION DE VIABILIDAD ─────
//

	auto comprobar_viabilidad = [&generados, &mejor_solucion, destino](nodo_bateria actual) {
		bool es_viable = true;

		if (actual.bateria_restante < 0){
			es_viable = false;
		}
		else {
			for (const auto elemento: generados) {
				if (	elemento.st.fila == actual.node.st.fila
					&&  elemento.st.columna == actual.node.st.columna
					&& 	elemento.st.orientacion == actual.node.st.orientacion
					//&&	elemento.zapatillas == actual.zapatillas
					//&& 	elemento.bikini == actual.bikini
					&& 	elemento.bateria >= actual.bateria_restante) {

					es_viable = false;
					break;
				}
			}
		}

		return es_viable;
	};


	// Para reducir el número de elementos a comprobar en la búsqueda, optimizamos la inserción
	auto insercion_optimizada = [&generados](nodo_bateria actual) {
		auto elemento = generados.begin();
		for (elemento; elemento != generados.end(); elemento++) {
			if (	elemento->st.fila == actual.node.st.fila && elemento->st.columna == actual.node.st.columna
				&&  elemento->st.orientacion == actual.node.st.orientacion
				/* && 	elemento->zapatillas == actual.zapatillas && elemento->bikini == actual.bikini */) {

					break;
			}
		}


		if (elemento == generados.end()) {
			generados.insert({actual.node.st, actual.bateria_restante, actual.zapatillas, actual.bikini});
		}
		else if (elemento->bateria < actual.bateria_restante && actual.bateria_restante >= 0) {
			generados.erase(elemento);
			generados.insert({actual.node.st, actual.bateria_restante, actual.zapatillas, actual.bikini});
		}
	};


//
// ──────────────────────────────────────────── BUCLE PRINCIPAL DEL ALGORITMO ─────
//

	nodo node;
	node.st = origen;
	node.secuencia.clear();

	nodo_bateria actual = {node, sensor.bateria, false, false};
	a_expandir.push(actual);

	while (!a_expandir.empty()) {
		/*
		 	Estructura de desarrollo de los hijos:
			Plantear un nuevo nodo con los datos del actual. Cambiar orientación o dar paso hacia adelante.
			Estos procesos gastan batería. Por tanto, dependiendo de lo que haga, restar el consumo correspondiente.

			Debemos tener en cuenta que podríamos haber cogido las zapatillas o el bikini. Si es así,
			el consumo se reducirá, y se debe actualizar su presencia en los nodos.
		*/

		a_expandir.pop();
		insercion_optimizada(actual);

		if (	actual.node.st.fila == destino.fila && actual.node.st.columna == destino.columna
			&&  actual.bateria_restante >= 0 && mejor_solucion.bateria_restante < actual.bateria_restante) {

			mejor_solucion = actual;
			cout << "Nueva solución encontrada:" << actual.node.st.fila << ", " << actual.node.st.columna << ". Batería: "<< actual.bateria_restante << ". Por expandir: " << a_expandir.size() << ". Generados: " << generados.size() << endl;
		}
		else if (actual.bateria_restante > 0) {

		//
		// ────────────────────────────────────────────── HIJO DERECHA ─────
		//

			nodo_bateria hijo_TR = actual;

			hijo_TR.node.st.orientacion  = (hijo_TR.node.st.orientacion + 1)%4;
			hijo_TR.bateria_restante    -= calcular_costo_bateria(hijo_TR.node.st, actTURN_R, hijo_TR.zapatillas, hijo_TR.bikini);
			hijo_TR.bateria_restante     = min(hijo_TR.bateria_restante, 3000);

			if (comprobar_viabilidad(hijo_TR)) {
				hijo_TR.node.secuencia.push_back(actTURN_R);
				a_expandir.push(hijo_TR);
			}

		//
		// ──────────────────────────────────────────── HIJO IZQUIERDA ─────
		//

			nodo_bateria hijo_TL = actual;

			hijo_TL.node.st.orientacion  = (hijo_TL.node.st.orientacion + 3)%4;
			hijo_TL.bateria_restante    -= calcular_costo_bateria(hijo_TL.node.st, actTURN_L, hijo_TL.zapatillas, hijo_TL.bikini);
			hijo_TL.bateria_restante     = min(3000, hijo_TL.bateria_restante);

			if (comprobar_viabilidad(hijo_TL)) {
				hijo_TL.node.secuencia.push_back(actTURN_L);
				a_expandir.push(hijo_TL);
			}



		//
		// ──────────────────────────────────────────── HIJO DELANTERO ─────
		//


			nodo_bateria hijo_forward = actual;

			hijo_forward.bateria_restante -= calcular_costo_bateria(hijo_forward.node.st, actFORWARD, hijo_forward.zapatillas, hijo_forward.bikini);
			hijo_forward.bateria_restante  = min(3000, hijo_forward.bateria_restante);


			if (!HayObstaculoDelante(hijo_forward.node.st) && comprobar_viabilidad(hijo_forward)) {
				hijo_forward.node.secuencia.push_back(actFORWARD);

				if (mapaResultado[hijo_forward.node.st.fila][hijo_forward.node.st.columna] == 'K')
					hijo_forward.bikini = true;
				if (mapaResultado[hijo_forward.node.st.fila][hijo_forward.node.st.columna] == 'D')
					hijo_forward.zapatillas = true;
				if (mapaResultado[hijo_forward.node.st.fila][hijo_forward.node.st.columna] == 'X') {
					while (hijo_forward.bateria_restante < 3000) {
						hijo_forward.node.secuencia.push_back(actIDLE);
						hijo_forward.bateria_restante += 10;
					}

					hijo_forward.bateria_restante = min(3000, hijo_forward.bateria_restante);
				}

				a_expandir.push(hijo_forward);
			}
		}

		if (!a_expandir.empty()) {
			actual = a_expandir.top();
		}
	}

	if (mejor_solucion.bateria_restante >= 0) {
		cout << "Cargando plan\n";
		plan = mejor_solucion.node.secuencia;

		cout << "Longitud del plan:" << plan.size() << endl;
		PintaPlan(plan);

		VisualizaPlan(origen, plan);

		return true;
	}
	else {
		cout << "F en el chat, no he encontrado ruta\n";
		return false;
	}
}

//
// ──────────────────────────────────────────────────── IV ──────────
//   :::::: A   S T A R : :  :   :    :     :        :          :
// ──────────────────────────────────────────────────────────────
//


void ComportamientoJugador::actualizar_mapaResultado (Sensores sensor) {
	size_t k = 0;

	for(size_t i = 0; i < 4; i++) {
		for(size_t j = 0; j <= i*2; j++ ) {
			switch (sensor.sentido) {
				case 0:
					if (sensor.posC - i + j < 100 && mapaResultado[sensor.posF - i][sensor.posC - i + j] == '?') {
						mapaResultado[sensor.posF - i][sensor.posC - i + j] = sensor.terreno[k];
						cuenta_nuevos_descub++;
					}

					k++;
					break;

				case 1:
					if (sensor.posC - i + j < 100 && sensor.posC + i < 100 && mapaResultado[sensor.posF - i + j][sensor.posC + i] == '?') {
						mapaResultado[sensor.posF - i + j][sensor.posC + i] = sensor.terreno[k];
						cuenta_nuevos_descub++;
					}

					k++;
					break;

				case 2:
					if (sensor.posF + i < 100 && sensor.posC + i - j < 100 && mapaResultado[sensor.posF + i][sensor.posC + i - j] == '?') {
						mapaResultado[sensor.posF + i][sensor.posC + i - j] = sensor.terreno[k];
						cuenta_nuevos_descub++;
					}

					k++;
					break;

				case 3:
					if (sensor.posF + i - j < 100 && mapaResultado[sensor.posF + i - j][sensor.posC - i] == '?') {
						mapaResultado[sensor.posF + i - j][sensor.posC - i] = sensor.terreno[k];
						cuenta_nuevos_descub++;
					}

					k++;
					break;
			}
		}
	}
}

pair<int, int> ComportamientoJugador::hay_powerups_cerca (Sensores sensor) {
	pair<int, int> destino (-1, -1);
	int k = 0;

	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j <= i*2; j++ ) {
			bool condicion_entrada = (sensor.terreno[k] == 'K' && !bikini) || (sensor.terreno[k] == 'D' && !zapatillas) || sensor.terreno[k] == 'X';

			switch (sensor.sentido) {
				case 0:
					if (condicion_entrada) {
						destino.first = sensor.posF - i;
						destino.second = sensor.posC - i + j;
					}

					break;

				case 1:
					if (condicion_entrada) {
						destino.first = sensor.posF - i + j;
						destino.second = sensor.posC + i;
					}

					break;

				case 2:
					if (condicion_entrada) {
						destino.first = sensor.posF + i;
						destino.second = sensor.posC + i - j;
					}

					break;

				case 3:
					if (condicion_entrada) {
						destino.first = sensor.posF + i - j;
						destino.second = sensor.posC - i;
					}

					break;
			}

			k++;
		}
	}

	// Si lo estoy pisando, ignorarlo
	if (destino.first == sensor.posF && destino.second == sensor.posC) {
		destino.first = -1;
		destino.second = -1;
	}

	return destino;
}


class estado_A: public estado {
public:
	double coste = 0;
};

class nodo_A: public nodo {
public:
	estado_A st;
	double coste_hasta_aqui = 0;
	bool   zapatillas       = false;
	bool   bikini           = false;
};

bool ComportamientoJugador::A_estrella(const estado &origen, const estado &destino, list<Action> &plan, const Sensores &sensor) {
	cout << "Calculando ruta\n";
	plan.clear();

	auto ordenar_prioridades = [](const nodo_A n1, const nodo_A n2) {
		return n1.coste_hasta_aqui > n2.coste_hasta_aqui;
	};

	priority_queue<nodo_A, vector<nodo_A>, decltype(ordenar_prioridades)> a_expandir (ordenar_prioridades);
	set<estado, ComparaEstados> generados;

	nodo_A actual;

	actual.st.fila        = origen.fila;
	actual.st.columna     = origen.columna;
	actual.st.orientacion = origen.orientacion;
	actual.zapatillas     = zapatillas;
	actual.bikini         = bikini;

	actual.secuencia.clear();

	a_expandir.push(actual);

	double peso_h = 1;
	double peso_g = 1;

	if (bikini && zapatillas) {
		peso_h = 2;
	}

	while (!a_expandir.empty() && (actual.st.fila != destino.fila || actual.st.columna != destino.columna)) {
		a_expandir.pop();
		generados.insert(actual.st);

//
// ───────────────────────────────────────────────────────────── HIJO DERECHA ─────
//

		nodo_A hijo_TR = actual;
		hijo_TR.st.orientacion = (hijo_TR.st.orientacion + 1)%4;

		int coste_TR =
			  peso_h * distancia(hijo_TR.st.fila, hijo_TR.st.columna, destino.fila, destino.columna)
			+ peso_g * calcular_costo_bateria(hijo_TR.st, actTURN_R, hijo_TR.zapatillas, hijo_TR.bikini);

		hijo_TR.coste_hasta_aqui = coste_TR;
		hijo_TR.st.coste = coste_TR;

		if (generados.find(hijo_TR.st) == generados.end()) { // Si no está el hijo en la lista de generados

			hijo_TR.secuencia.push_back(actTURN_R);
			a_expandir.push(hijo_TR);
		}

//
// ─────────────────────────────────────────────────────────── HIJO IZQUIERDA ─────
//

		nodo_A hijo_TL = actual;
		hijo_TL.st.orientacion = (hijo_TL.st.orientacion + 3)%4;
		int coste_TL =
			  peso_h * distancia(hijo_TL.st.fila, hijo_TL.st.columna, destino.fila, destino.columna)
			+ peso_g * calcular_costo_bateria(hijo_TL.st, actTURN_L, hijo_TL.zapatillas, hijo_TL.bikini);

		hijo_TL.coste_hasta_aqui = coste_TL;
		hijo_TL.st.coste = coste_TL;

		if (generados.find(hijo_TL.st) == generados.end()) { // Si no está el hijo en la lista de generados
			hijo_TL.secuencia.push_back(actTURN_L);
			a_expandir.push(hijo_TL);
		}

//
// ─────────────────────────────────────────────────────── HIJO HACIA DELANTE ─────
//

		nodo_A hijo_forward = actual;
		// Comrpobar que en la casilla de delante del origen no hay un NPC
		int sig_fila_origen = origen.fila, sig_col_origen = origen.columna;
		switch (origen.orientacion) {
			case 0: sig_fila_origen--; break;
			case 1: sig_col_origen++; break;
			case 2: sig_fila_origen++; break;
			case 3: sig_col_origen--; break;
		};

		int coste_forward =
			  peso_h * distancia(hijo_forward.st.fila, hijo_forward.st.columna, destino.fila, destino.columna)
			+ peso_g * calcular_costo_bateria(hijo_forward.st, actFORWARD, hijo_forward.zapatillas, hijo_forward.bikini);

		hijo_forward.coste_hasta_aqui = coste_forward;
		hijo_forward.st.coste = coste_forward;

		if ( !HayObstaculoDelante(hijo_forward.st)) {
			if ( generados.find(hijo_forward.st) == generados.end() ) {

				hijo_forward.secuencia.push_back(actFORWARD);
				a_expandir.push(hijo_forward);
			}
		}

		if (!a_expandir.empty()) {
			actual = a_expandir.top();

			if (actual.st.fila == sig_fila_origen && actual.st.columna == sig_col_origen && hay_NPC_delante(sensor) && !a_expandir.empty()) {
				a_expandir.pop();
				actual = a_expandir.top();
			}
		}
	}

	cout << "Búsqueda terminada\n";

	if (actual.st.fila == destino.fila && actual.st.columna == destino.columna) {
		cout << "Cargando plan\n";
		plan = actual.secuencia;

		cout << "Longitud del plan:" << plan.size() << endl;
		PintaPlan(plan);

		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "F en el chat, no hemos encontrado plan\n";
		return false;

	}
}



// Sacar por la términal la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}



void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else if (*it == actTURN_L){
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
