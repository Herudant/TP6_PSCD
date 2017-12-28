//============================================================================
// Name        : vallaSecundaria.h
// Author      : Eduardo Alonso Monge
// Description : practica 4 de pscd, vallaSecundaria.h
//============================================================================

#ifndef VALLASECUNDARIA_H_
#define VALLASECUNDARIA_H_



#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "peticiones.h"

using namespace std;

const int MAXNUMS = 20;
const int COSTES = 2;


class VallaSecundaria {

	public:
		VallaSecundaria();
		void solicitar(const string img, const int tmp, int& costeOp,
				time_t& horaEspera);
		void datosImagen(string& img, time_t& tmp, const int numVent);
		void avisar(time_t tmp,const int numVent);

	private:
		mutex mtx;

		queue<Peticion> peticiones1,
						peticiones2;

		time_t  tiempoEspera1,
				tiempoEspera2;

		condition_variable 	espera1,
							espera2;

};

#endif /* VALLASECUNDARIA_H_ */
