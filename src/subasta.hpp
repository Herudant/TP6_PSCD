//============================================================================
// File: subasta.hpp
// Authors:	Alonso Monge Eduardo
//					Bentue Blanco Miguel
//					Carreras Aguerri Pablo Noel
// Date:	Enero 2018
//============================================================================


#ifndef SUBASTA_H_
#define SUBASTA_H_


#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>

using namespace std;

class Subasta{

	public:
		//* ------- CONSTRUCTORES ------------------------------------------------*/
		Subasta();

		//* ------- GETTER -------------------------------------------------------*/
		int getPrecio_subasta();
		int getNum_subastas();
		int getTiempo_subasta();
		bool getActiva();

		//* ------- FUNCIONES ----------------------------------------------------*/
		//inicia una subasta
		void iniciarSubasta(const int precio, const int tiempo);
		//Un proceso intenta acceder una subasta y se bloquea hasta que se inicia la
		//subasta deseada
		int entrarSubasta(const int i);
		//cierra la subasta actual
		void cerrarSubasta();
		//cuando la ultima subasta acabe pone a true la variable de fin de subastas
		void cerrarServicio();
		//realiza una puja en la subasta actual
		int pujar(const int id, const int precio);
		//duerme un proceso
		void dormirLider();
		//avisa al subastador de que debe continuar
		void avisarSubastador();
		//despierta un proceso
		void despertar();
		//indica si se ha alcanzado un numero de subastas determinado
		bool maxSubastas(const int max);

	private:
		//* ------- ATRIBUTOS ----------------------------------------------------*/
		mutex mtx;
		bool fin_servicio;	//True -> servicio terminado y por tanto no se permiten
												//mas subastas
    bool activa; //False -> Subasta sin empezar True -> Subasta empezada
    int precio_subasta; //precio maximo por el que va la subasta
    int id_ganador;			//id del ganador en ese momento de la subasta
    int num_subastas;		//numero de subastas realizadas hasta el momento
		int tiempo_subasta; //tiempo que tiene el cliente para pujar en la subasta
		bool ganador_pendiente;	//indica si hay un ganador de la subasta en este
														//momento

		condition_variable espera, espera_ganador;//var.cond para quedarte bloqueado
};

#endif
