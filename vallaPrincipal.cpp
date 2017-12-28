//============================================================================
// Name        : vallaPrincipal.cpp
// Author      : Eduardo Alonso Monge
// Description : practica 4 de pscd, archivo fuente vallaPrincipal.cpp
//============================================================================

#include "vallaPrincipal.h"


VallaPrincipal::VallaPrincipal() {

	tiempoEspera = time(0);

}

// Si el buffer esta lleno error, precio = -1
void VallaPrincipal::solicitar(const string img, const int tmp,
		int& costeOperacion, time_t& horaEspera) {
	unique_lock<mutex> lck(mtx);

	if (peticiones.size() < MAXNUM) {
		costeOperacion = COSTE * tmp; // Devuelve el coste de la valla por el tiempo
		horaEspera = tiempoEspera;

		tiempoEspera += tmp;

		Peticion peticionAux;
		peticionAux.ruta = img;
		peticionAux.tiempo = tmp;
		peticiones.push(peticionAux);

		esperaImg.notify_one();

	} else {
		costeOperacion = -1;

	}

}

void VallaPrincipal::datosImagen(string& rutaImg, time_t& tiempoImg) {

	unique_lock<mutex> lck(mtx);

	while (peticiones.empty()) {
		esperaImg.wait(lck);
	}

	if (!peticiones.empty()) {
		Peticion peticionAux;

		peticionAux = peticiones.front();
		peticiones.pop();

		rutaImg = peticionAux.ruta;
		tiempoImg = peticionAux.tiempo;

	}


}

void VallaPrincipal::avisar(time_t tmp) {
	unique_lock<mutex> lck(mtx);

	tiempoEspera = tiempoEspera - tmp;
}

