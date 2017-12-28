//============================================================================
// Name        : vallaPrincipal.h
// Author      : Eduardo Alonso Monge
// Description : practica 4 de pscd, vallaPrincipal.h
//============================================================================

#ifndef VALLAPRINCIPAL_H_
#define VALLAPRINCIPAL_H_



#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "peticiones.h"

using namespace std;

const int MAXNUM = 20;
const int COSTE = 1;


class VallaPrincipal {

	public:
		VallaPrincipal();
		void solicitar(const string img,const int tmp,
									int& costeOp, time_t& horaEspera);
		void datosImagen(string& img,int& tmp);
		void avisar(int tmp);

	private:

		mutex mtx;
		queue<Peticion> peticiones;
		time_t tiempoEspera;

		condition_variable esperaImg;

};


#endif /* VALLAPRINCIPAL_H_ */
