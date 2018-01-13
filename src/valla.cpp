//============================================================================
// Name        :
// Author      :
// Description :
//============================================================================

#include "valla.hpp"

using namespace std;

Valla::Valla() {
	this -> tiempoEspera = 0;
	this -> num_peticiones = 0;
	this -> tiempo_total = 0;
	this -> tiempo_imagenes_mostradas = 0;
	this -> num_imagenes = 0;
	this -> n_libres = MAX_VENTANAS;
	this -> fin_servicio = false;
	for(auto i : this->ventanas_libres){
		i = true;
	}
}

void Valla::addPeticion(const string img, const int tmp) {
	unique_lock<mutex> lck(mtx);

	if (this -> peticiones.size() < MAXNUM && !this -> fin_servicio) {

		if(tiempoEspera < time(0))
			tiempoEspera = time(0);

		this -> tiempoEspera += tmp;

		// Encola una tupla {img, tiempo}
		auto peticion = make_tuple(img, tmp);
		this -> peticiones.push(peticion);

		// Actualiza datos para estadisticas
		this -> num_peticiones++;
		this -> tiempo_total+=tmp;

		// Notifica que hay una nueva petición
		espera_peticion.notify_one();
	}

}

tuple<int, string, int> Valla::atenderPeticion() {
	unique_lock<mutex> lck(mtx);
	cout << "\t\t\tatendiendo peticion";
	while(this->peticiones.empty())
		espera_peticion.wait(lck);
	cout << "\t\t\thay peticiones y n_libres = " << this->n_libres << endl;

	while(this->n_libres == 0)
		espera_ventana.wait(lck);
	cout << "\t\t\tatendiendo n_libres =" << this->n_libres << endl;
	int n_ventana;
	for(int i = 0; i < MAX_VENTANAS; ++i){
		if(this->ventanas_libres[i]){
			this->ventanas_libres[i] = false;
			n_ventana = i;
			break;
		}
	}

	// Saco la peticion de la cola y actualizo variables estadisticas
	auto peticion = peticiones.front();
	this -> peticiones.pop();
	this -> n_libres--;


	// Devuelvo el número de ventana y la peticion
	auto ret = make_tuple(n_ventana, get<0>(peticion), get<1>(peticion));
	return ret;

}

void Valla::finPeticion(const int tmp, const int n_ventana) {
	unique_lock<mutex> lck(mtx);
	this -> ventanas_libres[n_ventana] = true;
	this -> n_libres++;
	this -> num_imagenes++;
	this -> tiempoEspera = tiempoEspera - tmp;
	this -> tiempo_imagenes_mostradas += tmp;
	espera_ventana.notify_one();
	if(this -> fin_servicio && this -> peticiones.empty()){
		espera_fin.notify_one();
	}
}

void Valla::cerrarServicio(){
	unique_lock<mutex> lck(mtx);
	this -> fin_servicio = true;
	while( !this -> peticiones.empty()){
		espera_fin.wait(lck);
	}
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
