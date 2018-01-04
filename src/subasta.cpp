//============================================================================
// Name        : .h
// Author      :
// Description :
//============================================================================

#include "subasta.hpp"

using namespace std;

Subasta::Subasta(){
  this -> activa = false;
  this -> fin_servicio = false;
  this -> num_subastas = 0;
  this -> precio_subasta = 0;
  this -> tiempo_espera = 0;
  this -> id_ganador = -1;
}


/*
 *
 */
void Subasta::iniciarSubasta(const int precio, const int tiempo){
  unique_lock<mutex> lck(mtx);
  if(!this-> fin_servicio){
    this -> activa = true;
    this -> precio_subasta = precio;
    this -> tiempo_espera = tiempo;
    this -> id_ganador = -1;
    espera.notify_all();
  }
}

void Subasta::cerrarSubasta(){
  unique_lock<mutex> lck(mtx);
  this -> activa = false;
  this -> precio_subasta = 0;
  this -> tiempo_espera = 0;
  this -> num_subastas++;
  espera.notify_all();
}

void Subasta::cerrarServicio(){
  unique_lock<mutex> lck(mtx);
  if( this -> activa )
    espera.wait(lck);
  this -> fin_servicio = true;
}
/*
 *
 */
int Subasta::entrarSubasta(){
  unique_lock<mutex> lck(mtx);
  while(!activa){
    espera.wait(lck);
  }
  return this -> precio_subasta;
}

/*
 *
 */
int Subasta::pujar(const int id, const int precio){
  unique_lock<mutex> lck(mtx);
  if(activa){
    if(precio > precio_subasta){
      this -> id_ganador = id;
      this -> precio_subasta = precio;
        espera.notify_one();
    }
  }
  return (id != id_ganador) ? this -> precio_subasta : -1;
}

void Subasta::dormirLider(){
  unique_lock<mutex> lck(mtx);
  espera.wait(lck);
}

void Subasta::maxSubastas(const int max){
  unique_lock<mutex> lck(mtx);
  return (max == this -> num_subastas);
}
/*
 *
 */
int Subasta::getPrecio_subasta(){
  unique_lock<mutex> lck(mtx);
	return this -> precio_subasta;
}
/*
 *
 */
int Subasta::getNum_subastas(){
  unique_lock<mutex> lck(mtx);
	return this -> num_subastas;
}
/*
 *
 */
time_t Subasta::getTiempo_espera(){
  unique_lock<mutex> lck(mtx);
	return this -> tiempo_espera;
}

/*
 *
 */
bool Subasta::getActiva(){
  unique_lock<mutex> lck(mtx);
  return this -> activa;
}
