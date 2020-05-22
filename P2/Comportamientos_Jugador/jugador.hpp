#ifndef COMPORTAMIENTOJUGADOR_H
#define COMPORTAMIENTOJUGADOR_H

#include "comportamientos/comportamiento.hpp"

#include <list>

struct estado {
  int fila;
  int columna;
  int orientacion;
};

class ComportamientoJugador : public Comportamiento {
  public:
    ComportamientoJugador(unsigned int size) : Comportamiento(size) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayPlan = false;
    }
    ComportamientoJugador(std::vector< std::vector< unsigned char> > mapaR) : Comportamiento(mapaR) {
      // Inicializar Variables de Estado
      fil = col = 99;
      brujula = 0; // 0: Norte, 1:Este, 2:Sur, 3:Oeste
      destino.fila = -1;
      destino.columna = -1;
      destino.orientacion = -1;
      hayPlan = false;
    }
    ComportamientoJugador(const ComportamientoJugador & comport) : Comportamiento(comport){}
    ~ComportamientoJugador(){}

    Action think(Sensores sensores);
    int interact(Action accion, int valor);
    void VisualizaPlan(const estado &st, const list<Action> &plan);
    ComportamientoJugador * clone(){return new ComportamientoJugador(*this);}

  private:
    // Declarar Variables de Estado
    int fil, col, brujula;
    bool zapatillas = false, bikini = false;
    estado actual, destino;
    list<Action> plan;
    //bool powerup_localizado = false;

    Action ultimaAccion;
    bool hayPlan;
    bool recargas_calculadas_CU = false;
    bool recalcular_nuevos_descub = false;
    int cuenta_nuevos_descub = 0;

    bool acabo_de_calcular = false;
    bool powerup_avistado = false;
    const int limite_inf_bateria = 2800;

    // Métodos privados de la clase
    int calcular_costo_bateria(estado state, Action accion, bool zapatillas, bool bikini);

    bool pathFinding(Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan);
    bool pathFinding_Costo_Uniforme(const Sensores sensor, const estado &origen, const estado &destino, list<Action> &plan);
    bool A_estrella(const estado &origen, const estado &destino, list<Action> &plan, const Sensores & sensor);

    void actualizar_mapaResultado (Sensores sensor);
    pair<int, int> hay_powerups_cerca (Sensores sensor);
    bool hay_NPC_delante(Sensores sensor);

    void PintaPlan(list<Action> plan);
    bool HayObstaculoDelante(estado &st);

};

#endif
