//============================================================================
// Name        : valla.h
// Author      :
// Description :
//============================================================================

#ifndef VALLA_H_
#define VALLA_H_

#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <tuple>
#include <fstream>

using namespace std;

const int MAXNUM = 20;
const int MAX_VENTANAS = 2;

class Valla {

	public:
		Valla();
		int getNum_peticiones();
		int getNum_imagenes();
		int getTiempo_estimado();
		time_t getTiempo_total();
		time_t getTiempo_imagenes_mostradas();

		/* --------- Funciones ---------------------------------------------------*/

		// Añade una peticion {img, tmp} a la cola de peticiones
		void addPeticion(const string img,const int tmp);

		// Devuelve la peticion de la cola de peticiones y el número de ventana
		// libre en el que se debe mostrar {n_ventana, img, tmp}
		tuple<int, string, int> atenderPeticion();

		// Avisa de la finalización de la petición y libera la ventana
		void finPeticion(const int tmp, const int n_ventana);


		void cerrarServicio();


		// Funciones de escritura en exclusión mutua
		void write(string msg);
		void write(string msg, ofstream &fs);




	private:
		mutex mtx;
		mutex mtx_write;
		bool ventanas_libres[MAX_VENTANAS];
		bool fin_servicio;
		int n_libres;
		queue<tuple<string, int>> peticiones;	// cola con tuplas {img, tiempo}
		time_t tiempoEspera;

		// Estadisticas
		int num_peticiones;
		int num_imagenes;
		time_t tiempo_total;
		time_t tiempo_imagenes_mostradas;

		condition_variable espera_peticion, espera_ventana, espera_fin;

};


#endif /* VALLA_H_ */
