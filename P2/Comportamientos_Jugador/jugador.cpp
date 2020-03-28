#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <queue>


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
		hijo_TL.st.orientacion = (hijo_TL.st.orientacion - 1)%4;
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
	}
}


bool ComportamientoJugador::pathFinding_Costo_Uniforme(const Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan) {
	cout << "Calculando ruta\n";
	plan.clear();

	// Idea: meter en una priority queue el nodo a desarrollar junto con el nivel de batería
	// que ha consumido el plan hasta ahora. Automáticamente se ordenará todo. Solo falta saber cómo hacer eso.
	// Quizás una estructura específica y le meto un comparador? O puedo hacerlo a pelo?
	// Andrés del futuro, lo dejo en tus manos. Tú puedes. You can do the thing.
	auto compara_nivel_bateria = [](){return ;};

	//priority_queue<nodo, int> a_expandir;
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
