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
const int COSTE = 1;


class Valla {

	public:
		Valla();
		void mostrar(const string img);
		void solicitar(const string img,const int tmp,
									int& costeOp, time_t& horaEspera);
		void datosImagen(string& img,int& tmp);
		void avisar(int tmp);

	private:
		mutex mtx;
		queue<tuple<string, int>> peticiones;
		time_t tiempoEspera;

		condition_variable esperaImg;

};


#endif /* VALLA_H_ */
