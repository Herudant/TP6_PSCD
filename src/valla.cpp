//============================================================================
// Name        : valla.cpp
// Author      : Eduardo Alonso Monge
// Description : practica 4 de pscd, archivo fuente valla.cpp
//============================================================================

#include "valla.h"


Valla::Valla() {
	this -> tiempoEspera = 0;
	this -> num_peticiones = 0;
	this -> tiempo_total = 0;
	this -> tiempo_imagenes_mostradas = 0;
	this -> num_imagenes = 0;
}

// Si el buffer esta lleno error, precio = -1
void Valla::solicitar(const string img, const int tmp) {
	unique_lock<mutex> lck(mtx);

	if (peticiones.size() < MAXNUM) {

		if(tiempoEspera < time(0))
			tiempoEspera = time(0);

		this -> tiempoEspera += tmp;

		// Encola una tupla {img, tiempo}
		tuple<string, int> peticion;
		make_tuple(img, tmp);
		this -> peticiones.push(peticion);

		// Actualiza datos para estadisticas
		this -> num_peticiones++;
		this -> tiempo_total+=tmp;

		// Notifica que hay una nueva petición
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
		this -> peticiones.pop();

		rutaImg = get<0>(peticion);
		tiempoImg = get<1>(peticion);
		this -> num_imagenes++;
	}

}

void Valla::avisar(int tmp) {
	unique_lock<mutex> lck(mtx);
	this -> tiempoEspera = tiempoEspera - tmp;
	this -> tiempo_imagenes_mostradas += tmp;
}

int Valla::getNum_peticiones(){
	return this -> num_peticiones;
}

int Valla::getNum_imagenes(){
	return this -> num_imagenes;
}

time_t Valla::getTiempo_total(){
	return this -> tiempo_total;
}

time_t Valla::getTiempo_imagenes_mostradas(){
	return this -> tiempo_imagenes_mostradas;
}

int Valla::getTiempo_estimado(){
	int ret = 0;
	queue<tuple<string, int>> aux = peticiones;
	for (int i = 0; i < aux.size(); i++) {
		// Saco la componente tiempo de la tupla de peticiones
		int tiempo_peticion = get<1>(aux.front());
		aux.pop();
		ret+= tiempo_peticion;
	}
	return ret;
}
