///============================================================================
// Name        : subasta.cpp
// Author      :
// Description :
//============================================================================

#include "subasta.hpp"

using namespace std;
/*
 * Constructor Subasta
 */
Subasta::Subasta(){
  this -> activa = false;
  this -> fin_servicio = false;
  this -> num_subastas = 0;
  this -> precio_subasta = 0;
  this -> tiempo_subasta = 0;
  this -> id_ganador = -1;
  this -> ganador_pendiente = true;
}


/*
 * Inicia la subasta con un precio y un tiempo
 */
void Subasta::iniciarSubasta(const int precio, const int tiempo){
  unique_lock<mutex> lck(mtx);
  if(!this-> fin_servicio){
    this -> activa = true;
    this -> precio_subasta = precio;
    this -> tiempo_subasta = tiempo;
    this -> id_ganador = -1;
    espera.notify_all();
  }
}
/*
 * Despierta la subasta
 */
void Subasta::despertar(){
  unique_lock<mutex> lck(mtx);
  espera.notify_one();
}

/*
 * Cierra la subasta mediante sus variables privadas
 */
void Subasta::cerrarSubasta(){
  unique_lock<mutex> lck(mtx);
  this -> activa = false;
  this -> precio_subasta = 0;
  this -> tiempo_subasta = 0;
  this -> num_subastas++;
  espera.notify_all();

  while(this -> ganador_pendiente && this -> id_ganador != -1){
    espera_ganador.wait(lck);
  }
  this -> ganador_pendiente = true;
}
/*
 * Cierra el servicio de Subasta
 */
void Subasta::cerrarServicio(){
  unique_lock<mutex> lck(mtx);
  if( this -> activa )
    espera.wait(lck);
  this -> fin_servicio = true;
}
/*
 * Entra en la subasta
 */
int Subasta::entrarSubasta(const int i){
  unique_lock<mutex> lck(mtx);
  while(!activa || i != this->num_subastas){
    espera.wait(lck);
  }
  return this -> precio_subasta;
}

/*
 * El cliente identificado con su id, y su puja, precio, realiza una puja
 *  en la Subasta
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
/*
 * Bloquea al cliente que haya realizado la puja más alta en ese momento
 */
void Subasta::dormirLider(){
  unique_lock<mutex> lck(mtx);
  espera.wait(lck);
}
/*
 * Avisa al Subastador
 */
void Subasta::avisarSubastador(){
  unique_lock<mutex> lck(mtx);
  ganador_pendiente = false;
  espera_ganador.notify_one();
}

/*
 * Devuelve true si y solo si max es mayor o igual que el numero de subastas
 * realizadas por el momento
 */
bool Subasta::maxSubastas(const int max){
  unique_lock<mutex> lck(mtx);
  return (max >= this -> num_subastas);
}
/*
 * Devuelve el precio maximo por el que va la subasta
 */
int Subasta::getPrecio_subasta(){
  unique_lock<mutex> lck(mtx);
	return this -> precio_subasta;
}
/*
 * Devuelve el numero de subastas realizadas desde la inicialización
 */
int Subasta::getNum_subastas(){
  unique_lock<mutex> lck(mtx);
	return this -> num_subastas;
}
/*
 * Devuelve el tiempo que tiene el cliente para pujar en la subasta
 */
int Subasta::getTiempo_subasta(){
  unique_lock<mutex> lck(mtx);
	return this -> tiempo_subasta;
}

/*
 * Devuelve true si y solo si la subasta se encuentra activa
 */
bool Subasta::getActiva(){
  unique_lock<mutex> lck(mtx);
  return this -> activa;
}
