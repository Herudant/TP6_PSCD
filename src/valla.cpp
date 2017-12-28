//============================================================================
// Name        : valla.cpp
// Author      : Eduardo Alonso Monge
// Description : practica 4 de pscd, archivo fuente valla.cpp
//============================================================================

#include "valla.h"


Valla::Valla() {


}

// Si el buffer esta lleno error, precio = -1
void Valla::solicitar(const string img, const int tmp, time_t& horaEspera) {
	unique_lock<mutex> lck(mtx);

	if (peticiones.size() < MAXNUM) {

		if(tiempoEspera < time(0))
			tiempoEspera = time(0);

		horaEspera = tiempoEspera;
		tiempoEspera += tmp;

		// Encola una tupla {img, tiempo}
		tuple<string, int> peticion;
		make_tuple(img, tmp);
		peticiones.push(peticion);

		esperaImg.notify_one();

	}

}

void Valla::mostrar(string& rutaImg, int& tiempoImg) {

	unique_lock<mutex> lck(mtx);

	while(peticiones.empty()) {
		esperaImg.wait(lck);
	}

	if (!peticiones.empty()) {
		tuple<string, int> peticion;
		peticion = peticiones.front();
		peticiones.pop();

		rutaImg = get<0>(peticion);
		tiempoImg = get<1>(peticion);
	}


}

void Valla::avisar(int tmp) {
	unique_lock<mutex> lck(mtx);

	tiempoEspera = tiempoEspera - tmp;
}
