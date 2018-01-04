//*****************************************************************
// File:   Servidor.cpp
// Author:
// Date:
//*****************************************************************


/*---------------  Librerias y ficheros --------------------------------------*/

#include <iostream>					// Librerias
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <signal.h>
#include <atomic>
#include "valla.h"					// Ficheros
#include "Socket.hpp"
#include "CImg.h"
#include "ImageDownloader.hpp"

using namespace std;
/*----------------------------------------------------------------------------*/

/*---------------  Funciones privadas ----------------------------------------*/

// Atiende peticion Cliente
void dispatcher(int client_fd, Socket& socket, Subasta& subasta, Valla& valla);

// Gestor de las vallas publicitarias
void gestor_valla();

// Gestor de subastas, crea subastas que duran un periodo de tiempo aleatorio
void subastador(Subasta& subasta);

// Espera a recibir el mensaje de finalización para avisar a los threads
// de la finalización ordenada del servicio
void avisarFin();

// Imprime una imagen en una ventana durante un tiempo
void printImage(const string ruta, time_t tiempo, cimg_library::CImgDisplay& v);

// Captura señal de interrupcion para evitar cerrar el servidor
void handler(int n);

// Envía el mensaje al cliente asociado al socket
void send_msg(const int client_fd, Socket& socket, const string msg);

// Recibe y devuelve el mensaje del cliente asociado al socket
string recv_msg(const int client_fd, Socket& socket);
/*----------------------------------------------------------------------------*/

/*--------------- Variables globales del sistema  ----------------------------*/

const int MESSAGE_SIZE = 4001;      // mensajes de no más 4000 caracteres
const int MAX_SUBASTAS = 2;		 		  // numero máximo de subastas del servicio
const int _WIDTH = 800;						  // limites de la valla
const int _HEIGHT = 800;

Valla valla;

ofstream fs("log_servidor.log");		// fichero de log

atomic<bool> FIN_SERVICIO = false;	// indica la terminación del servicio
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	if(argc != 2){
		cout << "Error:  Se esperaba ./Servidor Puerto\n";
		exit(1);
	}

	char MENS_FIN[]="END OF SERVICE";
	// Puerto donde escucha el proceso servidor
  int SERVER_PORT = atoi(argv[1]);

	// Creación del socket con el que se llevará a cabo
	// la comunicación con el servidor.
	Socket socket(SERVER_PORT);

	// Bind
	int socket_fd =socket.Bind();
	if (socket_fd == -1) {
		cerr << "Error en el bind: " << strerror(errno) << endl;
		exit(1);
	}

	// Capturamoss la señal para evitar fallos
	signal(SIGINT, handler);

	// Descargamos la imagen por defecto
	char ruta[100] = "imgs/default.jpg";
	char cURL[500] = "https://upload.wikimedia.org/wikipedia/commons/thumb/5/5b/Insert_image_here.svg/1280px-Insert_image_here.svg.png";

	ImageDownloader downloader;
	downloader.downloadImage(cURL, ruta);

	/*--------------- Lanzamos modulos del sistema -----------------------------*/
	Subasta subasta;
	Valla valla;

	// Lanzamos el thread de valla
	thread valla(&gestor_valla);
	valla.detach();

	// Lanzamos el thread avisarFin
	thread p(&avisarFin,socket_fd, ref(socket), ref(subasta));
	p.detach();

	// Lanzar thread subasta
	thread subastador(&subastador, ref(subasta));
	subastador.detach();

	// Lanzar thread administrador
	thread administrador(&administrador, ref(subasta), ref(valla));
	administrador.detach();

	/*--------------------------------------------------------------------------*/

	// Listen
	int max_connections = 4;
	int error_code = socket.Listen(max_connections);
	if(error_code == -1) {
		cerr << "Error en el listen: " << strerror(errno) << endl;
		// Cerramos el socket
		socket.Close(socket_fd);
		exit(1);
	}

	// Bucle de aceptacion de peticiones
	int id = 0;
	while (1) {
		// Accept
		int client_fd = socket.Accept();
		cout << "-------- ABRIENDO SOCKET: " << client_fd << endl;
		if(client_fd == -1) {
			cerr << "Error en el accept: " << strerror(errno) << endl;
			// Cerramos el socket
			socket.Close(socket_fd);
			exit(1);
		}

		// lanzamos el thread para atender al cliente
		thread t(&dispatcher, client_fd, ref(socket), ref(subasta), ref(valla), id);
		t.detach();
		id++;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// FUNCIONES PRIVADAS /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


// Atiende peticion Cliente
void dispatcher(int client_fd, Socket& socket, Subasta& subasta, Valla& valla, const int id)
{
	int error_code;
	char MENS_FIN[]="END OF SERVICE";

	// Buffer para recibir el mensaje
	int length = 100;
	string buffer;

	bool out = false; // Inicialmente no salir del bucle

	// Mientras dura el servicio de subastas
	while(!FIN_SERVICIO){

		// Esperamos a que se inicie una subasta
		int ultimo_precio;
		ultimo_precio = subasta.entrar_subasta();

		while(!out && subasta.getActiva()) {

			//notificar al cliente de la subasta
			send_msg(client_fd, ref(socket), to_string(ultimo_precio));

			// Recibimos el mensaje del cliente (su puja)
			buffer = recv_msg(client_fd, ref(socket));

			// Si recibimos "END OF SERVICE" --> Fin de la comunicación
			if(buffer == MENS_FIN_PUJA)
				out = true; // Salir del bucle
			else {
				cout << "Mensaje recibido por: " << client_fd << " -> '" << buffer << "\n";

				// Cliente hace puja, si devuelve -1 soy ganador
				precio_ganador = subasta.pujar(id, atoi(buffer.c_str()));

				// Enviamos la respuesta
				string resp;
				resp = (precio_ganador == -1) ? "GANADOR#"  + buffer :
																				"PERDEDOR#" + to_string(precio_ganador);
				send_msg(client_fd, ref(socket), resp);

				// Si soy el ganador
				if(precio_ganador == -1){
					subasta.dormirLider();
					int precio = subasta.getPrecio_subasta();

					if(subasta.getActiva()){
						// Me despiertan porque han superado mi puja
						resp = "NUEVO_GANADOR#" + to_string(precio);
						send_msg(client_fd, ref(socket), resp);
					}
					else{
						// Me despiertan porque acaba la y soy el ganador
						resp = "FIN_GANADOR#"  + to_string(precio);

						// Envio mensaje de ganador al cliente
						send_msg(client_fd, ref(socket), resp);

						// Recibo la URL de la valla y solicito una petición al gestor_valla
						buffer = recv_msg(client_fd, ref(socket));
						valla.addPeticion(buffer, subasta.getTiempo_espera());
					}
				}
		}
	}

	// Cerramos el socket del cliente
	cout << "-------- CERRANDO SOCKET: " << client_fd << endl;
	error_code = socket.Close(client_fd);
	if(error_code == -1){
		cerr << "Error cerrando el socket del cliente: " << strerror(errno) << endl;
	}
}

// Gestor de la valla
void gestor_valla(Valla& valla)
{
	ImageDownloader downloader;
	int tiempo, n_valla;
	string URL;

	char ruta[100] = "../imgs/image.jpg";

	// VALLA_1
	cimg_library::CImg<unsigned char> img_("../imgs/default.jpg");
	cimg_library::CImgDisplay valla_0(img_.resize(_WIDTH, _HEIGHT),"VALLA 1");
	valla_1.resize(_WIDTH, _HEIGHT);
	valla_1.move(0, 0); // Esquina superior izquierda

	// VALLA_2
	cimg_library::CImgDisplay valla_1(img_.resize(_WIDTH, _HEIGHT),"VALLA 2");
	valla_2.resize(_WIDTH, _HEIGHT);
	valla_2.move(0, 850); // Esquina superior izquierda

	cimg_library::CImgDisplay& valla_aux();
	while (1) {
		// Atiende petición, recibe {n_valla, URL, tiempo}
		tie(n_valla, URL, tiempo) = valla.atenderPeticion();

		//Descargamos imagen
		downloader.downloadImage(URL.c_str(), ruta);

		msg = "\n\t----------------------------------------------------\n" +
					"\t\tMOSTRANDO VENTANA (" + to_string(n_valla) + "): " +
					 to_string(tiempo) + " segundos, " + URL +
					"\n\t----------------------------------------------------\n";

		valla.write(msg, fs);

		valla_aux = (n_valla == 0) ? ref(valla_0) : ref(valla_1);
		printImage(ruta, tiempo, valla_aux);

		//Avisamos de la finalizacion y mostramos valla por defecto
		printImage("imgs/default.jpg", 0, valla_aux);
		valla.finPeticion(tiempo, n_valla);
	}

}

// Imprime una imagen en una ventana durante un tiempo
void printImage(const string ruta, time_t tiempo, cimg_library::CImgDisplay& v)
{
	char rutaIMG[100];
	cimg_library::CImg<unsigned char> img_sec(ruta.c_str());
	v.render(img_sec.resize(_WIDTH, _HEIGHT));
	v.paint(); // Repintar nueva imagen en la valla
	if(tiempo > 0)
		this_thread::sleep_for(chrono::milliseconds(tiempo*1000));

}


// Muestra información del sistema en un fichero de log
// y se encarga de la terminación ordenada del servicio
void administrador(Subasta& subasta, Valla& valla)
{
	while(!FIN_SERVICIO || !subasta.maxSubastas(MAX_SUBASTAS)){
		string msg;
		time_t tiempo_total, tiempo_contratado, tiempo_imagenes;
		int num_peticiones, num_imagenes;
		// Mostrar información histórica del sistema (num imagenes y tiempo)
		tiempo_imagenes = valla.getTiempo_imagenes_mostradas();
		num_imagenes = valla.getNum_imagenes();
		msg = "------ INFORMACION HISTORICA DEL SISTEMA -------------------------" +
					"\n\tNumero de imagenes mostradas: " + to_string(num_imagenes)    +
					"\n\tTiempo de imagenes mostradas: " + to_string(tiempo_imagenes) +
					"\n---------------------------------------------------------------\n";
		valla.write(msg, ref(fs));

		// Mostrar información del estado del sistema (num peticiones y tiempo contratado)
		num_peticiones = valla.getNum_peticiones();
		tiempo_contratado = valla.getTiempo_estimado();
		tiempo_total = valla.getTiempo_total();
		msg = "------ INFORMACION DEL ESTADO DEL SISTEMA ------------------------" +
					"\n\tNumero de peticiones : " + to_string(num_peticiones)    +
					"\n\tTiempo contrado      : " + to_string(tiempo_contratado) +
					"\n\tTiempo total         : " + to_string(tiempo_total)      +
					"\n---------------------------------------------------------------\n";
		valla.write(msg, ref(fs));

	}

	// Iniciar la terminación ordenada del servicio
	cout << "Esperando a la finalización de las subastas....\n";
	subasta.cerrarServicio();
	cout << "Subastas finalizadas\n";

	cout << "Esperando a la finalización de las peticiones restantes....\n";
	valla.cerrarServicio();
	cout << "Peticiones finalizadas\n";

  cout << "Cerrando socket....\n";
	int error_code = socket.Close(socket_fd);
	if(error_code == -1)
		cerr << "Error cerrando el socket: " << strerror(errno) << endl;

	cout << "Bye bye" << endl;
	exit(1);
}


// Gestor de subastas, crea subastas que duran un periodo de tiempo aleatorio
void subastador(Subasta& subasta)
{
	srand(time(NULL));
	while(!FIN_SERVICIO || !subasta.maxSubastas(MAX_SUBASTAS)){
		int precio_subasta = rand() % 200 + 5;
		int tiempo_subasta = rand() % 20  + 2;
		subasta.iniciarSubasta(precio_subasta, tiempo_subasta);
		int tiempo = rand() % 10 + 10;
		this_thread::sleep_for(chrono::milliseconds(tiempo*1000));
		subasta.cerrarSubasta();
	}
}



// Espera a recibir el mensaje de finalización para avisar a los threads
// de la finalización ordenada del servicio
void avisarFin()
{
	string mensaje;
	while(1){
		getline(cin,mensaje);
		if (mensaje == "END OF SERVICE"){
			FIN_SERVICIO = true;
			break;
		}
	}
}

// socket.send
void send_msg(const int client_fd, Socket& socket, const string msg)
{
	int send_bytes = socket.Send(client_fd, resp);
	if(send_bytes == -1) {
		cerr << "Error al enviar datos: " << strerror(errno) << endl;
		// Cerramos los sockets
		socket.Close(client_fd);
		exit(1);
	}
}

// socket.recv
string recv_msg(const int client_fd, Socket& socket)
{
	string buffer;
	// Recibimos el mensaje del cliente (su puja)
	int rcv_bytes = socket.Recv(client_fd, buffer, MESSAGE_SIZE);
	if(rcv_bytes == -1) {
		cerr << "Error al recibir datos: " << strerror(errno) << endl;
		socket.Close(client_fd);
	}
	return buffer;
}


void handler(int n)
{
	signal(SIGINT, handler);
	cout << "Para salir escribe 'END OF SERVICE' \n";
}
