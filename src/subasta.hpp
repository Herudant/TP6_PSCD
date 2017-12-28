//============================================================================
// Name        : .h
// Author      :
// Description :
//============================================================================

#ifndef SUBASTA_H_
#define SUBASTA_H_



#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "peticiones.h"

using namespace std;

class Subasta{

	public:
		Subasta();
    Subasta(const int precio, const int tiempo);
    void iniciarSubasta(const int precio, const int tiempo);  //entra el subastador
    int entrarSubasta(const int id);   //los threads que atienden espera a que se inicia
    void cerrarSubasta();
    int pujar(const int id, const int precio);
    void dormirLider();

    int getPrecio_subasta();
    int getNum_subastas();
    time_t getTiempo_espera();
    queue<Peticion> getPeticiones();
    bool getActiva();

		void solicitar(const string img,const int tmp,
									int& costeOp, time_t& horaEspera);
		void datosImagen(string& img,int& tmp);
		void avisar(int tmp);

	private:
		mutex mtx;

    bool activa; //False -> Subasta sin empezar True -> Subasta empezada
    int precio_subasta; //precio maximo por el que va la subasta
    int id_ganador;			//id del ganador en ese momento de la subasta
    int num_subastas;
		time_t tiempo_espera; //tiempo que tiene el cliente para pujar en la subasta

		condition_variable espera; //var.cond para quedarte bloqueado
};

#endif
