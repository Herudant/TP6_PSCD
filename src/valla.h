//============================================================================
// Name        : valla.h
// Author      : Eduardo Alonso Monge
// Description : practica 4 de pscd, valla.h
//============================================================================

#ifndef VALLA_H_
#define VALLA_H_



#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <tuple>

using namespace std;

const int MAXNUM = 20;

class Valla {

	public:
		Valla();
		void mostrar(string& rutaImg, int& tiempoImg);
		void solicitar(const string img,const int tmp);
		void datosImagen(string& img,int& tmp);
		void avisar(int tmp);
		int getNum_peticiones();
		int getNum_imagenes();
		int getTiempo_estimado();
		time_t getTiempo_total();
		time_t getTiempo_imagenes_mostradas();


	private:
		mutex mtx;
		queue<tuple<string, int>> peticiones;	// cola con tuplas {img, tiempo}
		time_t tiempoEspera;
		int num_peticiones;
		int num_imagenes;
		time_t tiempo_total;
		time_t tiempo_imagenes_mostradas;

		condition_variable esperaImg;

};


#endif /* VALLA_H_ */
