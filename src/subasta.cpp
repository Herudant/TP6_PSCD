//============================================================================
// Name        : .h
// Author      :
// Description :
//============================================================================

#include "subasta.hpp"

using namespace std;

Subasta::Subasta(){
  activa = false;
  num_subastas = 0;
  precio_subasta = 0;
  tiempo_espera = 0;
  id_ganador = -1;
}

Subasta::Subasta(const int precio, const int tiempo){
  activa = true;
  num_subastas = 0;
  precio_subasta = precio;
  tiempo_espera = tiempo;
  id_ganador = -1;
}

/*
 *
 */
void Subasta::iniciarSubasta(const int precio, const int tiempo){
  unique_lock<mutex> lck(mtx);
  activa = true;
  num_subastas++;
  precio_subasta = precio;
  tiempo_espera = tiempo;
  id_ganador = -1;
	espera.notify_all();
}

void Subasta::cerrarSubasta(){
  unique_lock<mutex> lck(mtx);
  activa = false;
  precio_min = 0;
  precio_subasta = 0;
  tiempo_espera = 0;
  espera.notify_one();
}

/*
 *
 */
int Subasta::entrarSubasta(){
  unique_lock<mutex> lck(mtx);
  while(!activa){
    espera.wait(lck);
  }
  return precio_min;
}

/*
 *
 */
int Subasta::pujar(const int id, const int precio){
  unique_lock<mutex> lck(mtx);
  if (activa){
    if(precio > precio_subasta){
        id_ganador = id;
        precio_subasta = precio;
        espera.notify_one();
    }
  }
  //Si activa -> devuelve precio_subasta
  //Si !activa -> devuelve -1 => eres el ganador de la subasta
  return (id != id_ganador) ? precio_subasta : -1;
}

void Subasta::dormirLider(){
  unique_lock<mutex> lck(mtx);
  espera.wait(lck);
}

/*
 *
 */
int Subasta::getPrecio_subasta(){
  unique_lock<mutex> lck(mtx);
	return precio_subasta;
}
/*
 *
 */
int Subasta::getNum_subastas(){
  unique_lock<mutex> lck(mtx);
	return num_subastas;
}
/*
 *
 */
time_t Subasta::getTiempo_espera(){
  unique_lock<mutex> lck(mtx);
	return tiempo_espera;
}

/*
 *
 */
bool Subasta::getActiva(){
  unique_lock<mutex> lck(mtx);
  return activa;
}
