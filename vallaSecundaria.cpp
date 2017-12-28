//============================================================================
// Name        : vallaSecundaria.cpp
// Author      : Eduardo Alonso Monge
// Description : practica 4 de pscd, Archivo fuente vallaSecundaria.cpp
//============================================================================

#include "vallaSecundaria.h"

VallaSecundaria::VallaSecundaria(){

	tiempoEspera1 = time(0);
	tiempoEspera2 = time(0);

}

void VallaSecundaria::solicitar(const string img, const int tmp,
									int& costeOperacion, time_t& horaEspera) {

	unique_lock<mutex> lck(mtx);
	if(peticiones1.size() == MAXNUMS && peticiones2.size() == MAXNUMS){
		costeOperacion = -1;
	}
	else{
		Peticion peticionAux;
		peticionAux.ruta = img;
		peticionAux.tiempo = tmp;

		costeOperacion = COSTES * tmp; // Devuelve el coste de la valla por el tiempo

		if(((tiempoEspera1 < tiempoEspera2) && peticiones1.size() != MAXNUMS )
											|| peticiones2.size() == MAXNUMS)
		{
			horaEspera = tiempoEspera1;
			tiempoEspera1+= tmp;
			peticiones1.push(peticionAux);
			espera1.notify_one();
		}
		else{
			horaEspera = tiempoEspera2;
			tiempoEspera2+= tmp;
			peticiones2.push(peticionAux);
			espera2.notify_one();
		}




	}

}

void VallaSecundaria::datosImagen(string& rutaImg, time_t& tiempoImg,const int numVentana) {

	unique_lock<mutex> lck(mtx);

	while (peticiones1.empty()){
		espera1.wait(lck);
	}

	while (peticiones2.empty()){
		espera2.wait(lck);
	}


	Peticion peticionAux;

	if(numVentana == 2 && !peticiones2.empty()){
		peticionAux = peticiones2.front();
		peticiones2.pop();


	}else if (numVentana == 1 && !peticiones1.empty() ){
		peticionAux = peticiones1.front();
		peticiones1.pop();

	}

	rutaImg = peticionAux.ruta;
	tiempoImg = peticionAux.tiempo;




}

void VallaSecundaria::avisar(time_t tmp, const int numVent){
	unique_lock<mutex> lck(mtx);

	if(numVent == 1){
		tiempoEspera1-= tmp;
	}
	else{
		tiempoEspera2-=tmp;
	}




}