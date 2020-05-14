# Documentación práctica 2 - Belkan

> Autor: Andrés Millán Muñoz


<!-- @import "[TOC]" {cmd="toc" depthFrom=2 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [Preámbulos](#preámbulos)
- [Búsqueda en anchura](#búsqueda-en-anchura)
- [Costo Uniforme](#costo-uniforme)
    - [Modificación del nodo](#modificación-del-nodo)
    - [Modificación de la estructura estado](#modificación-de-la-estructura-estado)
    - [Funciones auxiliares](#funciones-auxiliares)
    - [Algoritmo principal](#algoritmo-principal)
- [A* modificado](#a-modificado)
- [El método think](#el-método-think)
- [Algunos resultados](#algunos-resultados)
    - [mapa30.map](#mapa30map)
    - [mapa50.map](#mapa50map)
    - [mapa75.map](#mapa75map)

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

Quizás el algoritmo que requiere de más modificaciones. Para completar este nivel tenemos que encontrar el camino mediante el cual se llega al destino con el mayor nivel de batería posible. Por ello, podría ocurrir que debamos pararnos a recargar en alguna de las estaciones de recarga del mapa. El árbol de posibilidades será muy amplio.

La disposición básica del algoritmo es análoga a la de los dos anteriores. Sin embargo, necesitamos alterar las estructuras de datos para tener en cuenta tanto batería como zapatillas y bikini. No nos podemos basar en el sensor para esto.

Introducimos, por tanto, las siguientes:

#### Modificación del nodo
```C++
// Extensión de los nodos para contabilizar el gasto de energía.
struct nodo_bateria {
    nodo node;
    int bateria_restante;
    bool zapatillas;
    bool bikini;
};
```

Podría haberse realizado mediante una herencia. Sin embargo, decidí optar por esto.

#### Modificación de la estructura estado

```C++
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
```

Los dos operadores nos ayudarán a simplificar la definición del conjunto de nodos generados, así como realizar comparaciones entre estados. Además, esta estructura lleva asociada un hash en Costo Uniforme:

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
		case 'A': if (bikini) costo = 10; else costo = 100;
			break;
		case 'B': if (zapatillas) costo = 5; else costo = 50;
			break;
		case 'T': costo = 2;
            break;
		case '?': costo =  5; // Suposición - FIXME
			break;
		case 'X': costo = -10;
            break;
		default:  costo = 1;
			break;
	}

	return costo;
}
```

Podemos ver que las estaciones de recarga las tratamos mediante un decremento negativo, lo cual supondrá un incremento.

Para optimizar las inserciones, así como la comprobación de la viabilidad de un nodo, usaremos las siguientes lambdas dentro del algoritmo:

```C++
auto prioridad_nodo_bateria = [](const nodo_bateria& n1, const nodo_bateria& n2) {
    return n1.bateria_restante < n2.bateria_restante;
};

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
```

#### Algoritmo principal
Finalmente, este es el algoritmo:

```C++
priority_queue<nodo_bateria, vector<nodo_bateria>, decltype(prioridad_nodo_bateria)> a_expandir(prioridad_nodo_bateria);

unordered_set<estado_bateria, hash_estado_bateria> generados;
nodo_bateria mejor_solucion = {};


nodo node;
node.st = origen;
node.secuencia.clear();

nodo_bateria actual = {node, sensor.bateria, false, false};
a_expandir.push(actual);

while (!a_expandir.empty()) {
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
        hijo_TR.node.st.orientacion = (hijo_TR.node.st.orientacion + 1)%4;
        hijo_TR.bateria_restante -= calcular_costo_bateria(hijo_TR.node.st, actTURN_R, hijo_TR.zapatillas, hijo_TR.bikini);

        if (actual.bateria_restante > mejor_solucion.bateria_restante && comprobar_viabilidad(hijo_TR)) {
            hijo_TR.node.secuencia.push_back(actTURN_R);
            a_expandir.push(hijo_TR);
        }

    //
    // ──────────────────────────────────────────── HIJO IZQUIERDA ─────
    //

            // Análogo; corto para reducir el tamaño del PDF

    //
    // ──────────────────────────────────────────── HIJO DELANTERO ─────
    //


        nodo_bateria hijo_forward = actual;
        hijo_forward.bateria_restante -= calcular_costo_bateria(hijo_forward.node.st, actFORWARD, hijo_forward.zapatillas, hijo_forward.bikini);


        if (!HayObstaculoDelante(hijo_forward.node.st) && actual.bateria_restante > mejor_solucion.bateria_restante && comprobar_viabilidad(hijo_forward)) {
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

                hijo_forward.bateria_restante = max(3000, hijo_forward.bateria_restante);
            }

            a_expandir.push(hijo_forward);
        }
    }

    if (!a_expandir.empty()) {
        actual = a_expandir.top();
    }

    // Cargar plan
}
```

## A* modificado

La metodología ahora cambia un poco. Hemos pasado de tener toda la información del mapa que nos permitía calcular planes perfectos, a necesitar deambular a ciegas. Por ello, necesitaremos actualizar la información conforme avancemos.

Usamos la siguiente función para actualizar el mapa:

```C++
void ComportamientoJugador::actualizar_mapaResultado (Sensores sensor) {
	size_t k = 0;

	for(size_t i = 0; i < 4; i++) {
		for(size_t j = 0; j <= i*2; j++ ) {
			switch (sensor.sentido) {
				case 0:
					if (sensor.posC - i + j < 100 && mapaResultado[sensor.posF - i][sensor.posC - i + j] == '?')
						mapaResultado[sensor.posF - i][sensor.posC - i + j] = sensor.terreno[k];

					k++;
					break;

				case 1:
					if (sensor.posC - i + j < 100 && sensor.posC + i < 100 && mapaResultado[sensor.posF - i + j][sensor.posC + i] == '?')
						mapaResultado[sensor.posF - i + j][sensor.posC + i] = sensor.terreno[k];

					k++;
					break;

				case 2:
					if (sensor.posF + i < 100 && sensor.posC + i - j < 100 && mapaResultado[sensor.posF + i][sensor.posC + i - j] == '?')
						mapaResultado[sensor.posF + i][sensor.posC + i - j] = sensor.terreno[k];

					k++;
					break;

				case 3:
					if (sensor.posF + i - j < 100 && mapaResultado[sensor.posF + i - j][sensor.posC - i] == '?')
						mapaResultado[sensor.posF + i - j][sensor.posC - i] = sensor.terreno[k];

					k++;
					break;
			}
		}
	}
}
```

Cuando estemos avanzando, nos podríamos encontrar NPCs delante. La siguiente función nos permitirá saber si hay alguno en nuestro camino:

```C++
bool ComportamientoJugador::hay_NPC_delante(Sensores sensor) {
	return sensor.superficie[2] == 'a';
}
```

El algoritmo que usaremos será una modificación de A* en el que tenga en cuenta la batería. Usaremos el planteamiento básico de Costo Uniforme para hacerlo. Pero, esta vez, no esperaremos a tener todo el árbol generado; sino que cortaremos en cuanto encontremos una solución. La heurística debe ser, por tanto, acertada.

La que usaremos será:

```C++
auto heuristica = [&destino](const nodo_bateria& n1, const nodo_bateria& n2) {
    return distancia(n2.node.st.fila, n2.node.st.columna, destino.fila, destino.columna) - 3 * n2.bateria_restante
                <
            distancia(n1.node.st.fila, n1.node.st.columna, destino.fila, destino.columna) - 3 * n1.bateria_restante;
};
```

En mi experiencia, el ajuste de la heurística puede cambiar radicalmente. Por ejemplo, usando de peso para la batería `*2`, hace que obtenga 96 resultados con la ejecución `./BelkanSG mapas/mapa30.map 1 4 9 12 1 3 3`, mientras que con `*3`, se obtienen 117.

A*, por tanto, queda así:

```C++
bool ComportamientoJugador::A_estrella(const estado &origen, const estado &destino, list<Action> &plan, const Sensores &sensor) {
	cout << "Calculando ruta\n";
	plan.clear();

	auto heuristica = [&destino](const nodo_bateria& n1, const nodo_bateria& n2) {
		return distancia(n2.node.st.fila, n2.node.st.columna, destino.fila, destino.columna) - 2 * n2.bateria_restante
					<
			   distancia(n1.node.st.fila, n1.node.st.columna, destino.fila, destino.columna) - 2 * n1.bateria_restante;
	};

	priority_queue<nodo_bateria, vector<nodo_bateria>, decltype(heuristica)> a_expandir (heuristica);
	set<estado_bateria> generados;

	nodo node;
	node.st = origen;
	node.secuencia.clear();
	nodo_bateria actual = {node, sensor.bateria, this->zapatillas, this->bikini};

	a_expandir.push(actual);

	while (!a_expandir.empty() && (actual.node.st.fila != destino.fila || actual.node.st.columna != destino.columna)) {
		a_expandir.pop();
		generados.insert({actual.node.st, actual.bateria_restante, actual.zapatillas, actual.bikini});

		nodo_bateria hijo_TR = actual;
		hijo_TR.node.st.orientacion = (hijo_TR.node.st.orientacion + 1)%4;
		hijo_TR.bateria_restante -= calcular_costo_bateria(hijo_TR.node.st, actTURN_R, hijo_TR.zapatillas, hijo_TR.bikini);

		if (generados.find({hijo_TR.node.st, hijo_TR.bateria_restante, hijo_TR.zapatillas, hijo_TR.bikini}) == generados.end()) {
			hijo_TR.node.secuencia.push_back(actTURN_R);
			a_expandir.push(hijo_TR);
		}


		nodo_bateria hijo_TL = actual;
		hijo_TL.node.st.orientacion = (hijo_TL.node.st.orientacion + 3)%4;
		hijo_TL.bateria_restante -= calcular_costo_bateria(hijo_TL.node.st, actTURN_L, hijo_TL.zapatillas, hijo_TL.bikini);

		if (generados.find({hijo_TL.node.st, hijo_TL.bateria_restante, hijo_TL.zapatillas, hijo_TL.bikini}) == generados.end()) {
			hijo_TL.node.secuencia.push_back(actTURN_L);
			a_expandir.push(hijo_TL);
		}


		nodo_bateria hijo_forward = actual;


		if ( !HayObstaculoDelante(hijo_forward.node.st)) {
			if ( generados.find({hijo_forward.node.st, hijo_forward.bateria_restante, hijo_forward.zapatillas, hijo_forward.bikini}) == generados.end() ) {
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

			// Comrpobar que en la casilla de delante del origen no hay un NPC
			int sig_fila_origen = origen.fila, sig_col_origen = origen.columna;
			switch (origen.orientacion) {
				case 0: sig_fila_origen--; break;
				case 1: sig_col_origen++; break;
				case 2: sig_fila_origen++; break;
				case 3: sig_col_origen--; break;
			};

			if (actual.node.st.fila == sig_fila_origen && actual.node.st.columna == sig_col_origen && hay_NPC_delante(sensor) && !a_expandir.empty()) {
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
	if (destino.fila == actual.fila && destino.columna == actual.columna) {
		hayPlan = false;
	}

	if (sensores.nivel == 4)
		actualizar_mapaResultado(sensores);

	actual.fila        = sensores.posF;
	actual.columna     = sensores.posC;
	actual.orientacion = sensores.sentido;
	fil                = actual.fila;
	col                = actual.columna;
	brujula            = actual.orientacion;
	destino.fila       = sensores.destinoF;
	destino.columna    = sensores.destinoC;

	if (mapaResultado[sensores.posF][sensores.posC] == 'D')
		zapatillas = true;
	if (mapaResultado[sensores.posF][sensores.posC] == 'K')
		bikini = true;

	// No se hacen funciones con efectos secundarios @profesores
	estado copia_actual = actual;

	if (	plan.empty()
		||  (plan.size() > 0 && plan.front() == actFORWARD && HayObstaculoDelante(copia_actual))	// He chocado con algo y no lo tenía previsto.
		||  hay_NPC_delante(sensores)) {

		hayPlan = pathFinding(sensores, actual, destino, plan);
	}


	if (sensores.nivel == 4) {
		pair<int, int> powerups_cercanos = hay_powerups_cerca(sensores);

		if (powerups_cercanos.first != -1 && powerups_cercanos.second != -1) {
			estado destino_temp;
			destino_temp.fila = powerups_cercanos.first;
			destino_temp.columna = powerups_cercanos.second;

			bool es_recarga = mapaResultado[powerups_cercanos.first][powerups_cercanos.second] == 'X';

			if (!es_recarga	|| (es_recarga && sensores.bateria < 1000)) {
				hayPlan = pathFinding(sensores, actual, destino_temp, plan);


				if (es_recarga) {
					int lvl_bateria = sensores.bateria;

					while (lvl_bateria < max(int(sensores.tiempo*2), 2000)) {
						plan.push_back(actIDLE);
						lvl_bateria += 10;
					}
				}
			}
		}
	}

	Action siguiente_accion = actIDLE;

	if (hayPlan && plan.size() > 0) {
		siguiente_accion = plan.front();
		plan.erase(plan.begin());
	}

  	return siguiente_accion;
}
```

- A cada tick de juego, se actualiza el mapa. Esto se hace por comodidad, pero es un gasto de recursos si ya se conoce lo que el sensor capta.
- Casi todas las variables nuevas añadidas a la cabecera se actualizan a cada paso.
- Se recalcula un plan bajo las siguientes condiciones:
  1. No teníamos plan.
  2. Se ha chocado con una pared.
  3. Se ha chocado con un NPC que se ha movido.
- En el nivel 4 detectamos si tenemos alguna casilla de recarga o de mejora cercana. Si se dan las condiciones adecuadas, se avanza hacia ellas. Esto es, no tenemos zapatillas o bikini; o nuestro nivel de batería cae por debajo de un límite. Si se cumple lo último, el enanito se quedará un rato esperando hasta que tenga un nivel aceptable de batería. Esta cantidad es un poco arbitraria, y lo he calculado empíricamente a ojo.


## Algunos resultados

Para terminar, presento algunos resultados obtenidos mediante la ejecución de `./BelkanSG mapas/mapa.map 1 4 9 12 1 3 3`:

#### mapa30.map
```
Tiempo Consumido: 0.3125
Nivel Final de Bateria: 1352
Colisiones: 0
Muertes: 0
Objetivos encontrados: 117
```
#### mapa50.map
```
Tiempo Consumido: 0.109375
Nivel Final de Bateria: 861
Colisiones: 0
Muertes: 0
Objetivos encontrados: 72
```

#### mapa75.map
```
Tiempo Consumido: 1.375
Nivel Final de Bateria: 0
Colisiones: 0
Muertes: 0
Objetivos encontrados: 23
```