//============================================================================
// File: valla.hpp
// Authors:	Alonso Monge Eduardo
//					Bentue Blanco Miguel
//					Carreras Aguerri Pablo Noel
// Date:	Enero 2018
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

		/* --------- CONSTRUCTORES -----------------------------------------------*/
		Valla();

		/* --------- GETTERS -----------------------------------------------------*/
		int getNum_peticiones();
		int getNum_imagenes();
		int getTiempo_estimado();
		time_t getTiempo_total();
		time_t getTiempo_imagenes_mostradas();

		/* --------- FUNCIONES ---------------------------------------------------*/

		// Añade una peticion {img, tmp} a la cola de peticiones
		void addPeticion(const string img,const int tmp);

		// Devuelve la peticion de la cola de peticiones y el número de ventana
		// libre en el que se debe mostrar {n_ventana, img, tmp}
		tuple<int, string, int> atenderPeticion();

		// Avisa de la finalización de la petición y libera la ventana
		void finPeticion(const int tmp, const int n_ventana);

		// Cierra el servicio y no permite atender nuevas peticiones
		void cerrarServicio();




	private:
		/* ---------  ATRIBUTOS --------------------------------------------------*/
		mutex mtx;
		bool ventanas_libres[MAX_VENTANAS];			// indica el estado de cada ventana
		bool fin_servicio;											// indica la finalización del servicio
		int n_libres;														// número de ventanas libre
		queue<tuple<string, int>> peticiones;		// cola con tuplas {img, tiempo}
		time_t tiempoEspera;

		// Estadisticas
		int num_peticiones;
		int num_imagenes;
		time_t tiempo_total;
		time_t tiempo_imagenes_mostradas;

		condition_variable espera_peticion, espera_ventana, espera_fin;

};


#endif /* VALLA_H_ */
