//============================================================================
// Name        : Vuelo.h
// Author      : Eduardo Alonso Monge
// Description : practica 5 de pscd
//============================================================================

#ifndef Vuelo_H_
#define Vuelo_H_



#include <iostream>
#include <string>
#include <mutex>
#include <list>
#include <string>
#include <condition_variable>

using namespace std;

const int NUM_FILAS = 10;
const int NUM_ASIENTOS = 4;

class Vuelo{

	public:
		Vuelo();
		int reservar(const int fila, const int columna);
		void liberar(const int fila, const int columna);
		list<int> get_libres();
		bool es_completo();


	private:
		mutex mtx;
		int num_asientos;
		bool asiento_libre[NUM_FILAS][NUM_ASIENTOS];
		condition_variable esperando;

};


#endif /* VALLAPRINCIPAL_H_ */
