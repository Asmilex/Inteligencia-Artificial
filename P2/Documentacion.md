# Documentación práctica 2 - Belkan

> Autor: Andrés Millán Muñoz


<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Preámbulos](#preámbulos)
- [Búsqueda en anchura](#búsqueda-en-anchura)
- [Costo Uniforme](#costo-uniforme)
    - [Estructuras de datos](#estructuras-de-datos)
    - [Funciones auxiliares](#funciones-auxiliares)
    - [Algoritmo principal](#algoritmo-principal)
- [Reto: A*](#reto-a)
- [El método think](#el-método-think)
- [Conclusiones](#conclusiones)

<!-- /code_chunk_output -->


## Preámbulos

En este documento haré una breve explicación sobre cada método implementado. Repasaré las estructuras de datos más relevantes, así como los principales algoritmos del programa.

## Búsqueda en anchura

Este es, sin duda, uno de los algoritmos más sencillos de implementar. Basándonos en la búsqueda en profundidad que ya venía implementada, es cuestión de hacer unas pequeñas modificaciones para adaptarlo a los requerimientos.

Con cambiar la pila de Profundidad por una cola, obtenemos búsqueda en anchura. El algoritmo, por tanto, quedaría de la siguiente forma:

```C++
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

    // Cargar plan
}
```

## Costo Uniforme

Quizás el algoritmo que requiere de más modificaciones. Para completar este nivel tenemos que encontrar el camino mediante el cual se llega al destino con el menor gasto de batería posible. Por ello, podría ocurrir que debamos pararnos a recargar en alguna de las estaciones de recarga del mapa. El árbol de posibilidades será muy amplio.

La disposición básica del algoritmo es análoga a la de los dos anteriores. Sin embargo, necesitamos alterar las estructuras de datos para tener en cuenta tanto batería como zapatillas y bikini. No nos podemos basar en el sensor para esto.

Introducimos, por tanto, las siguientes:

#### Estructuras de datos
```C++
class estado_A: public estado {
public:
	double coste = 0;
	bool zapatillas = false;
	bool bikini = false;
};

class nodo_A: public nodo {
public:
	estado_A st;
	double coste_hasta_aqui = 0;
	bool   zapatillas       = false;
	bool   bikini           = false;
};
```
Estas nuevas clases se usarán también para el nivel 4. Por otra parte, la estructura estado_bateria tendrá asociado un hash:

```C++
// Bateria -> B, fila -> f, columna -> c, zapatilla -> z, bikini -> b
// ffccBBBBzb
struct hash_estado_bateria {
    size_t operator()(const estado_bateria& state) const {
        return 		state.st.fila * 100000000 + state.st.columna * 1000000
                + 	state.bateria * 100
                +   state.zapatillas * 10 + state.bikini;
    }
};
```

Esto nos permite almacenar eficazmente en un mapa, en el que luego realizaremos tanto búsquedas como inserciones.


#### Funciones auxiliares
Asimismo, necesitaremos una función que contabilice el costo de cada acción:

```C++
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
```

Podemos ver que las estaciones de recarga las tratamos mediante un decremento negativo, lo cual supondrá un incremento.

Más tarde veremos el por qué del '?'.

En el algoritmo, nos apoyaremos de la siguiente función lambda para simplificar el algoritmo:

```C++
auto encontrar_en_expandir = [&a_expandir](const nodo_A& nodo) {
	for (multiset<nodo_A, prioridad_nodo_bateria>::const_iterator it = a_expandir.begin(); it != a_expandir.end(); ++it) {
		if ((*it).st.fila == nodo.st.fila && (*it).st.columna == nodo.st.columna && (*it).st.orientacion == nodo.st.orientacion && (*it).zapatillas == nodo.zapatillas && (*it).bikini == nodo.bikini) {
			return it;
		}
	}
	return a_expandir.end();
};
```

Finalmente, ordenaremos la cola de abiertos atendiendo al costo de una acción:

```C++
struct prioridad_nodo_bateria {
	bool operator()(const nodo_A& n1, const nodo_A& n2){
		return n1.coste_hasta_aqui < n2.coste_hasta_aqui;
	}
};
```


#### Algoritmo principal
Finalmente, este es el algoritmo:

```C++
bool ComportamientoJugador::pathFinding_Costo_Uniforme(const Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan) {
	cout << "Calculando ruta\n";
	plan.clear();



//
// ─────────────────────────────────────────────── ESTRUCTURAS DE DATOS CLAVE ─────
//


	multiset<nodo_A, prioridad_nodo_bateria> a_expandir;
	set<estado_A, ComparaEstados> generados;


//
// ──────────────────────────────────────────── BUCLE PRINCIPAL DEL ALGORITMO ─────
//

	nodo_A actual;

	actual.st.fila          = origen.fila;
	actual.st.columna       = origen.columna;
	actual.st.orientacion   = origen.orientacion;
	actual.coste_hasta_aqui = 0;
	actual.st.coste         = 0;
	actual.bikini           = false;
	actual.zapatillas       = false;

	a_expandir.insert(actual);

	while (!a_expandir.empty() && (actual.st.fila != destino.fila || actual.st.columna != destino.columna)) {
		a_expandir.erase(a_expandir.begin());
		generados.insert(actual.st);



	//
	// ────────────────────────────────────────────── HIJO DERECHA ─────
	//

		nodo_A hijo_TR = actual;
		hijo_TR.st.orientacion = (hijo_TR.st.orientacion + 1)%4;

		if (generados.find(hijo_TR.st) == generados.end()) {
			hijo_TR.coste_hasta_aqui += calcular_costo_bateria(hijo_TR.st, actTURN_R, hijo_TR.zapatillas, hijo_TR.bikini);
			hijo_TR.st.coste = hijo_TR.coste_hasta_aqui;

			// Buscar por si tenemos que expandirlo
			auto it = encontrar_en_expandir(hijo_TR);

			if (it != a_expandir.end() && hijo_TR.coste_hasta_aqui < it->coste_hasta_aqui) {
				// Sustituir
				a_expandir.erase(it);
				hijo_TR.secuencia.push_back(actTURN_R);
				a_expandir.insert(hijo_TR);

			}
			else {
				hijo_TR.secuencia.push_back(actTURN_R);
				a_expandir.insert(hijo_TR);
			}


		}

	//
	// ──────────────────────────────────────────── HIJO IZQUIERDA ─────
	//

		nodo_A hijo_TL = actual;

		hijo_TL.st.orientacion = (hijo_TL.st.orientacion + 3)%4;

		if (generados.find(hijo_TL.st) == generados.end()) {
			hijo_TL.coste_hasta_aqui += calcular_costo_bateria(hijo_TL.st, actTURN_L, hijo_TL.zapatillas, hijo_TL.bikini);
			hijo_TL.st.coste = hijo_TL.coste_hasta_aqui;

			// Buscar por si tenemos que expandirlo
			auto it = encontrar_en_expandir(hijo_TL);

			if (it != a_expandir.end() && hijo_TL.coste_hasta_aqui < it->coste_hasta_aqui) {
				// Sustituir
				a_expandir.erase(it);
				hijo_TL.secuencia.push_back(actTURN_L);
				a_expandir.insert(hijo_TL);

			}
			else {
				hijo_TL.secuencia.push_back(actTURN_L);
				a_expandir.insert(hijo_TL);
			}


		}



	//
	// ──────────────────────────────────────────── HIJO DELANTERO ─────
	//


		nodo_A hijo_forward = actual;



		if (!HayObstaculoDelante(hijo_forward.st) && generados.find(hijo_forward.st) == generados.end()) {
			hijo_forward.coste_hasta_aqui += calcular_costo_bateria(hijo_forward.st, actFORWARD, hijo_forward.zapatillas, hijo_forward.bikini);
			hijo_forward.st.coste = hijo_forward.coste_hasta_aqui;

			if (mapaResultado[hijo_forward.st.fila][hijo_forward.st.columna] == 'K')
				hijo_forward.bikini = true;
			if (mapaResultado[hijo_forward.st.fila][hijo_forward.st.columna] == 'D')
				hijo_forward.zapatillas = true;

			auto it = encontrar_en_expandir(hijo_forward);

			if (it != a_expandir.end()) {
				if (hijo_forward.coste_hasta_aqui < it->coste_hasta_aqui) {
					a_expandir.erase(it);
					hijo_forward.secuencia.push_back(actFORWARD);
					a_expandir.insert(hijo_forward);
				}
			}
			else {
				hijo_forward.secuencia.push_back(actFORWARD);
				a_expandir.insert(hijo_forward);
			}
		}


		if (!a_expandir.empty()) {
			actual = *(a_expandir.begin());
		}
	}

	// Cargar plan
}
```

## Reto: A*

La metodología ahora cambia un poco. Hemos pasado de tener toda la información del mapa que nos permitía calcular planes perfectos, a necesitar deambular a ciegas. Por ello, necesitaremos actualizar la información conforme avancemos.

Usamos la siguiente función para actualizar el mapa:

```C++
void ComportamientoJugador::actualizar_mapaResultado (Sensores sensor);
```

Cuando estemos avanzando, nos podríamos encontrar NPCs delante. La siguiente función nos permitirá saber si hay alguno en nuestro camino:

```C++
bool ComportamientoJugador::hay_NPC_delante(Sensores sensor) {
	return sensor.superficie[2] == 'a';
}
```

El algoritmo que usaremos será A*. Es un método de pathfinding muy famoso, sencillo de implementar y que produce resultados bastante buenos. Sabemos que el coste de una acción viene dado por $f = g + h$, donde $h$ es una heurística, y $g$ el peso de realizar la acción en el grafo.

La heurística que usaremos será la distancia Manhattan:

```C++
double distancia (int fila_0, int columna_0, int fila_destino, int columna_destino) {
	return abs(fila_0 - fila_destino) + abs(columna_0 - columna_destino);
};
```

A lo largo del recorrido obtendremos dos tipos de powerups: las zapatillas y el bikini. En mi experiencia, cuando se tienen ambas, el número de objetivos suele mejorar cuanto menos en cuenta se tiene la batería. Por tanto, usaremos unos pesos dinámicos en el cálculo del coste de los hijos:

```C++
double peso_h = 1;
double peso_g = 1;

if (bikini && zapatillas) {
	peso_h = 2;
}

//...

coste_TR =
	  peso_h * distancia(hijo.st.fila, hijo.st.columna, destino.fila, destino.columna)
	+ peso_g * calcular_costo_bateria(hijo.st, accion, hijo.zapatillas, hijo.bikini);
```


Volviendo a la función del cálculo del coste, destacamos la siguiente línea:

```C++

case '?':
	if (bikini && zapatillas)
		costo =  7;
	else if (bikini)
		costo = 10;
	else if (zapatillas)
		costo = 5;
	break;
```

La estrategia seguida es la misma. Cuando tengamos alguna de ellas, quitaremos peso a la casilla de interrogación, pues, nos encontremos lo que nos encontremos, el gasto será razonable. Por el contrario, si nos falta alguna de ellas, debemos andarnos con ojo, pues el coste puede llegar a ser de 100. Por último, si no tenemos ninguna de las dos, el coste será despreciable, para forzar a explorar el mapa.

El resto del algoritmo es similar a los otros. Lo único destacable es que, cuando nos encontramos delante de un enano, no desarrollamos el hijo que avanza. Para que no nos haga body blocking.

Finalmente, este es el algoritmo:

```C++
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

    //cargar plan
}
```

## El método think

Para terminar, A* debe venir acompañado de un algoritmo de decisión sólido. Recordemos que el pathfinding solo nos permite encontrar caminos; también debe ser capaz de decidir nuestro explorador.

Los primeros niveles se gestionan prácticamente solos: basta con decirle al pathfinding que ejecute el asociado al nivel pedido. En el nivel 4 podremos hacer otras gestiones más interesantes. Veamos la implementación:

```C++
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
```

Destaquemos las funcionalidades más relevantes, por orden de aparición:
- Cada 4 ticks de juego se desactiva una variable. Esto es un bugfix que evita situaciones en las que el algoritmo recalcula más de la cuenta cuando tiene un powerup delante.
- Si se descubren más de 15 posiciones en el mapa, las cuales antes teníamos marcadas con '?', se busca de nuevo la ruta. Esto permite sortear obstáculos y no ir a ciegas conforme investigamos.
- Cuando se llega a un destino se anulan algunas condiciones activas para la modificación de la ruta on the fly.
- Se recalcula el camino bajo alguna de las siguientes condiciones:
	1. No tenemos plan
	2. Tenemos plan, pero hemos chocado con un NPC o una pared.
	3. Hallamos una casilla especial. Esto incluye zapatillas, bikini y recarga con nuestros sensores. Cuando se consiguen las dos primeras, no se tienen más en cuenta en la ruta. No obstante, la última sí. Si captamos una recarga, iremos siempre que nuestro nivel de batería esté por debajo del valor notado con `const int limite_inf_bateria = 2800`.  Además, se espera hasta tener un nivel aceptable, calculado dinámicamente con respecto al tiempo restante.

## Conclusiones

En general, mis algoritmos intentan conseguir un balance entre agresividad y cautela. En las primeras fases de exploración, se procede con cautela buscando el camino de menos consumo. Sin embargo, conforme más herramientas se adquieren y menos tiempo queda restante, se intenta avanzar hacia el objetivo en línea recta dentro de lo razonable. Esto hace que en general se obtengan resultados adecuados.