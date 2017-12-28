//============================================================================
// Name        : Vuelo.cpp
// Author      : Eduardo Alonso Monge
// Description : practica 5 de pscd
//============================================================================

#include "vuelo.hpp"

Vuelo::Vuelo(){
	num_asientos = 0;
	for (int i = 0; i < NUM_FILAS; i++) {
		for (int j = 0; j < NUM_ASIENTOS; j++)
			asiento_libre[i][j] = true;
	}
}

/* Cada asiento esta representado por un número de tal forma que
 * la primera cifra del número indica la fila (0-9) y la segunda
 * cifra indica el asiento (0-3). Rango válido (0-39)
*/
int Vuelo::reservar(const int fila, const int columna){
  unique_lock<mutex> lck(mtx);
	int ret = -1;
	if((fila < NUM_FILAS && columna < NUM_ASIENTOS)) {
	  if(asiento_libre[fila][columna]){
			asiento_libre[fila][columna] = false;
			num_asientos++;
		  ret = (fila * 10) + columna;
		}
	}
  return ret;
}

list<int> Vuelo::get_libres(){
	list<int> ret;
	for (int i = 0; i < NUM_FILAS; i++) {
		for (int j = 0; j < NUM_ASIENTOS; j++){
			if(asiento_libre[i][j]){
				ret.push_back((j*10) + i);
			}
		}
	}
	return ret;
}

bool Vuelo::es_completo(){
	return (num_asientos == NUM_FILAS * NUM_ASIENTOS) ? true : false;
}

void Vuelo::liberar(const int fila, const int columna){
	unique_lock<mutex> lck(mtx);
	asiento_libre[fila][columna] = 1;
	num_asientos--;
}
