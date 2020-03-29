#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>
#include <unordered_set>


// Este es el método principal que debe contener los 4 Comportamientos_Jugador
// que se piden en la práctica. Tiene como entrada la información de los
// sensores y devuelve la acción a realizar.
Action ComportamientoJugador::think(Sensores sensores) {
	if (!hayPlan) {
        actual.fila        = sensores.posF;
        actual.columna     = sensores.posC;
        actual.orientacion = sensores.sentido;
        destino.fila       = sensores.destinoF;
        destino.columna    = sensores.destinoC;

		hayPlan = pathFinding(sensores, actual, destino, plan);
    }


	Action siguiente_accion = actIDLE;

	if (hayPlan && plan.size() > 0) {
		siguiente_accion = plan.front();
		plan.erase(plan.begin());
	}
	else {

	}

  	return siguiente_accion;
}


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
			return pathFinding_Costo_Uniforme(sensor, origen, destino, plan);
			break;

		case 4:
			cout << "Busqueda para el reto\n";
			return A_estrella(origen, destino, plan);
			break;
	}

	cout << "Comportamiento sin implementar\n";

	return false;
}


//
// ───────────────────────────── IMPLEMENTACION DE LA BUSQUEDA EN PROFUNDIDAD ─────
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


//
// ──────────────────────────────────────────────────────────── II ─────────
//   :::::: A L G O R I T M O S : :  :   :    :     :        :          :
// ──────────────────────────────────────────────────────────────────────
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

		nodo hijo_foward = actual;
		if (!HayObstaculoDelante(hijo_foward.st)) {
			if (generados.find(hijo_foward.st) == generados.end()) {
				hijo_foward.secuencia.push_back(actFORWARD);
				a_expandir.push(hijo_foward);
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

// ────────────────────────────────────────────────────────────────────────────────

int ComportamientoJugador::calcular_costo_bateria(estado state, Action accion,bool zapatillas, bool bikini) {
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
				costo = 500;

			break;

		case 'T':
			costo = 2;
			break;

		default:
			costo = 1;
			break;
	}

	return costo;
}


//
// ───────────────── ESTRUCTURAS DE DATOS NECESARIAS PARA UNIFORM COST SEARCH ─────
//


	struct nodo_bateria {
		nodo node;
		int bateria_restante;
		bool zapatillas;
		bool bikini;
	};


// ────────────────────────────────────────────────────────────────────────────────

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
	};

	// Estructura:
	// Bateria -> b, fila -> f, columna -> c, zapatilla -> z, bikini -> b
	// bbbbffcczb
	struct hash_estado_bateria {
		size_t operator()(const estado_bateria& state) const {
			return 		state.bateria * 1000000 + state.st.fila * 10000 +
					+ 	state.st.columna * 100 + state.zapatillas * 10 + state.bikini;
		}
	};

// ────────────────────────────────────────────────────────────────────────────────


//
// ─────────────────────────────────────────────────────────── COSTO UNIFORME ─────
//

bool ComportamientoJugador::pathFinding_Costo_Uniforme(const Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan) {
	cout << "Calculando ruta\n";
	plan.clear();

	// Idea: meter en una priority queue el nodo a desarrollar junto con el nivel de batería
	// que ha consumido el plan hasta ahora. Automáticamente se ordenará todo. Ir desarrollando los más prometedores.

	stack<nodo_bateria> a_expandir;

	nodo node;
	node.st = origen;
	node.secuencia.clear();

	nodo_bateria actual = {node, sensor.bateria, false, false};
	a_expandir.push(actual);

//
// ──────────────────────────────────── CONJUNTO ORDENADO DE NODOS ANALIZADOS ─────
//

	unordered_set<estado_bateria, hash_estado_bateria> generados;
	list<nodo_bateria> llegan_destino;

	bool es_redundante = false;


//
// ─────────────────────────────────────────────── OPTIMIZACION DE VIABILIDAD ─────
//

	auto comprobar_viabilidad = [&generados, &llegan_destino, destino](nodo_bateria actual) {
		bool es_viable = true;

		if (actual.bateria_restante < 0){
			es_viable = false;
		}
		else if (actual.node.st.fila == destino.fila && actual.node.st.columna == destino.columna) {
			// Comprobación rápida mediante soluciones.
			for (auto solucion: llegan_destino) {
				if (solucion.bateria_restante > actual.bateria_restante) {
					es_viable = false;
					break;
				}
			}
		}

		if (es_viable) {
			for (auto elemento: generados) {
				if (	elemento.st.fila == actual.node.st.fila
					&&  elemento.st.columna == actual.node.st.columna
					&& 	elemento.st.orientacion == actual.node.st.orientacion
					&&	elemento.zapatillas == actual.zapatillas
					&& 	elemento.bikini == actual.bikini
					&& 	elemento.bateria >= actual.bateria_restante) {

					es_viable = false;
					break;
				}
			}
		}

		return es_viable;
	};


//
// ──────────────────────────────────────────── BUCLE PRINCIPAL DEL ALGORITMO ─────
//

	while (!a_expandir.empty()) {		// Considero que hay que buscar el óptimo para batería. Debo analizar todos.
		/*
		 	Estructura de desarrollo de los hijos:
			Plantear un nuevo nodo con los datos del actual. Cambiar orientación o dar paso hacia adelante.
			Estos procesos gastan batería. Por tanto, dependiendo de lo que haga, restar el consumo correspondiente.

			Debemos tener en cuenta que podríamos haber cogido las zapatillas o el bikini. Si es así,
			el consumo se reducirá, y se debe actualizar su presencia en los nodos.
		*/
		a_expandir.pop();
		//cout << "Elementos restantes por analizar: " << a_expandir.size() << ". Nodo actual: (" << actual.node.st.fila << ", "
			//<< actual.node.st.columna << ", O=" << actual.node.st.orientacion << ", bateria = " << actual.bateria_restante << "z="
			//<< actual.zapatillas << ", b=" << actual.bikini << ")\n";

		generados.insert({actual.node.st, actual.bateria_restante, actual.bikini, actual.zapatillas});


		//acelerar soluciones mirando consumos de batería de las soluciones
		bool debe_analizarse = true;

		for (auto elemento: llegan_destino) {
			if (actual.bateria_restante <= elemento.bateria_restante)
				debe_analizarse = false;
		}

		if (	actual.node.st.fila == destino.fila && actual.node.st.columna == destino.columna
			&&  actual.bateria_restante >= 0) {

			llegan_destino.push_back(actual);
			cout << "Solución encontrada:" << actual.node.st.fila << ", " << actual.node.st.columna << ". Batería: "<< actual.bateria_restante <<endl;
		}
		else if (actual.bateria_restante > 0 && debe_analizarse) {

		//
		// ────────────────────────────────────────────── HIJO DERECHA ─────
		//

			nodo_bateria hijo_TR = actual;
			hijo_TR.node.st.orientacion = (hijo_TR.node.st.orientacion + 1)%4;
			hijo_TR.bateria_restante -= calcular_costo_bateria(hijo_TR.node.st, actTURN_R, hijo_TR.zapatillas, hijo_TR.bikini);

			if (comprobar_viabilidad(hijo_TR)) {
				//cout << "Nuevo nodo encontrado. Elementos restantes por expandir:" << a_expandir.size() << ". Elementos en el grupo de generados: " << generados.size() << endl;
				hijo_TR.node.secuencia.push_back(actTURN_R);
				a_expandir.push(hijo_TR);
			}

		//
		// ──────────────────────────────────────────── HIJO IZQUIERDA ─────
		//

			nodo_bateria hijo_TL = actual;
			hijo_TL.node.st.orientacion = (hijo_TL.node.st.orientacion + 3)%4;
			hijo_TL.bateria_restante -= calcular_costo_bateria(hijo_TL.node.st, actTURN_L, hijo_TL.zapatillas, hijo_TL.bikini);

			if (comprobar_viabilidad(hijo_TL)) {
				//cout << "Nuevo nodo encontrado. Elementos restantes por expandir:" << a_expandir.size() << ". Elementos en el grupo de generados: " << generados.size() << endl;
				hijo_TL.node.secuencia.push_back(actTURN_L);
				a_expandir.push(hijo_TL);
			}



		//
		// ──────────────────────────────────────────── HIJO DELANTERO ─────
		//


			nodo_bateria hijo_forward = actual;
			hijo_forward.bateria_restante -= calcular_costo_bateria(hijo_forward.node.st, actFORWARD, hijo_forward.zapatillas, hijo_forward.bikini);

			// Comprobar que merece la pena seguir expandiendo este nodo: si ha llegado a un consumo
			// mayor al de una posible solución, parar.
			bool expandir = true;

			for (auto solucion: llegan_destino) {
				if (solucion.bateria_restante > hijo_forward.bateria_restante) {
					expandir = false;
				}
			}

			if (expandir && !HayObstaculoDelante(hijo_forward.node.st)) {
				hijo_forward.node.secuencia.push_back(actFORWARD);

				if (mapaResultado[hijo_forward.node.st.fila][hijo_forward.node.st.columna] == 'K')
					hijo_forward.bikini = true;
				if (mapaResultado[hijo_forward.node.st.fila][hijo_forward.node.st.columna] == 'D')
					hijo_forward.zapatillas = true;

				a_expandir.push(hijo_forward);
			}
		}

		if (!a_expandir.empty()) {
			actual = a_expandir.top();
		}
	}

	cout << "Búsqueda terminada. Tamaño del conjunto de casillas analizadas: " << generados.size() << "\n";
	cout << "Tamaño del conjunto de caminos que han llegado al destino:" << llegan_destino.size() << endl;

	if (llegan_destino.size() > 0) {
		nodo_bateria camino = llegan_destino.front();

		for (auto elemento: llegan_destino) {
			if (elemento.bateria_restante > camino.bateria_restante)
				camino = elemento;
		}


		cout << "Cargando plan\n";
		plan = camino.node.secuencia;

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


bool ComportamientoJugador::A_estrella(const estado &origen, const estado &destino, list<Action> &plan) {

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
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}
