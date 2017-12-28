//*****************************************************************
// File:   Servidor.cpp
// Author: Eduardo Alonso
// Date:   noviembre 2015
// Coms:   Ejemplo de servidor con comunicación síncrona mediante sockets
//         Compilar el fichero "Makefile" asociado, mediante
//         "make".
//*****************************************************************

#include <iostream>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <signal.h>

#include "valla.h"

#include "Socket.hpp"
#include "CImg.h"
#include "ImageDownloader.hpp"

using namespace std;



const int MESSAGE_SIZE = 4001; //mensajes de no más 4000 caracteres

const int _WIDTH = 800;
const int _HEIGHT = 800;
const int SECUNDARIA_WIDTH = 800;
const int SECUNDARIA_HEIGHT = 400;
bool FIN_SERVICIO = false;
const int NUMVALLASEC = 2;
VallaSecundaria serv_secundario;
Valla serv_;



void imprImg(const string ruta,
	           time_t tiempo,
						 cimg_library::CImgDisplay& valla);
void crearImg(const string ruta,
						 	const int numVent,cimg_library::CImgDisplay& vallasec);
void vallaral();
void dispatcher(int client_fd, Socket& socket, Subasta& subasta, Valla& valla);
void subastador(Subasta& subasta);
void avisarFin(int socket_fd, Socket& socket, Subasta& subasta);
void handler(int n){
	signal(SIGINT, handler);
	cout << "Para salir escribe 'END OF SERVICE' \n";
}



//------------------------------------------------------------
int main(int argc, char *argv[]) {

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

	// --------------- Lanzamos modulos del sistema ------------------------------
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


	// ---------------------------------------------------------------------------

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

		// lanzamos el thread para atender al clliente
		thread t(&dispatcher, client_fd, ref(socket), ref(subasta), ref(valla), id);
		t.detach();
		id++;
	}

	return 0;
}



// Imprime en una ventana durante un tiempo la ruta de imagen dada
void imprImg(const string ruta, time_t tiempo, cimg_library::CImgDisplay& valla){
	char rutaIMG[100];
	strcpy(rutaIMG, ruta.c_str());
	cimg_library::CImg<unsigned char> img_sec(rutaIMG);
	valla.render(img_sec.resize(SECUNDARIA_WIDTH, SECUNDARIA_HEIGHT));
	valla.paint(); // Repintar nueva imagen en la valla
	if(tiempo > 0)
		this_thread::sleep_for(chrono::milliseconds(tiempo*1000));
}

// Thread que gestiona la ventana
void gestor_valla() {

	int tiempo;
	string URL;
	ImageDownloader downloader;
	char ruta[100] = "imgs/imagePral.jpg";
	char cURL[500] = "https://upload.wikimedia.org/wikipedia/commons/thumb/5/5b/Insert_image_here.svg/1280px-Insert_image_here.svg.png";

	// VALLA
	cimg_library::CImg<unsigned char> img_("imgs/default.jpg");
	cimg_library::CImgDisplay valla_(img_.resize(_WIDTH, SECUNDARIA_WIDTH),"VALLA ");
	valla_.resize(_WIDTH, SECUNDARIA_WIDTH);
	valla_.move(0, 0); // Esquina superior izquierda


	while (1) {

		//Datos imagen a mostrar
		serv_.datosImagen(URL, tiempo);

		//Descargamos imagen
		strcpy(cURL, URL.c_str());
		downloader.downloadImage(cURL, ruta);


		cout << "\n\t\t ---------------------------------------\n";
		cout << "\t\t MOSTRANDO VENTANA : "  << to_string(tiempo) << " segundos, " << URL << endl;
		cout << "\t\t ---------------------------------------\n";
		imprImg(ruta, tiempo, valla_);
		imprImg("imgs/default.jpg", 0, valla_);

		//Avisamos de la finalizacion
		serv_.avisar(tiempo);

	}

}


void crearImg(const string ruta, const int numVent,
		cimg_library::CImgDisplay& vallasec) {
	char rutaIMG[100];
	char nombreVent[100];
	string aux = " VALLA SEC " + to_string(numVent);
	strcpy(rutaIMG, ruta.c_str());
	strcpy(nombreVent, aux.c_str());

	cimg_library::CImg<unsigned char> img_sec(rutaIMG);

	cimg_library::CImgDisplay valla_sec(img_sec.resize(SECUNDARIA_WIDTH, SECUNDARIA_HEIGHT), nombreVent);
	valla_sec.resize(SECUNDARIA_WIDTH, SECUNDARIA_HEIGHT);
	vallasec = valla_sec;

}

void dispatcher(int client_fd, Socket& socket, Subasta& subasta, Valla& valla, const int id){
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
						// Me despiertan porque acaba la puja y soy el ganador
						resp = "FIN_GANADOR#"  + to_string(precio);

						// Envio mensaje de ganador al cliente
						send_msg(client_fd, ref(socket), resp);

						// Recibo la URL de la valla y solicito una petición al gestor_valla
						buffer = recv_msg(client_fd, ref(socket));
						valla.solicitar(buffer, subasta.getTiempo_espera());
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


// Espera a recibir el mensaje de finalización para cerrar el servidor
void avisarFin(int socket_fd, Socket& socket){
	string mensaje;
	while(1){
		getline(cin,mensaje);
		if (mensaje == "END OF SERVICE"){
			int error_code = socket.Close(socket_fd);
			if(error_code == -1)
				cerr << "Error cerrando el socket del servidor: " << strerror(errno) << endl;

			// Mensaje de despedida
			cout << "Bye bye" << endl;
			exit(1);
		}
	}
}

void administrador(Subasta& subasta, Valla& valla){

}

void subastador(Subasta& subasta){
	srand(time(NULL));
	while(!FIN_SERVICIO){
		int precio_subasta = rand() % 200;
		int tiempo_subasta = rand() % 20;
		subasta.iniciarSubasta(precio_subasta, tiempo_subasta);
		int tiempo = rand() % 10 + 10;
		this_thread::sleep_for(chrono::milliseconds(tiempo*1000));
		subasta.cerrarSubasta();
	}
}

void send_msg(const int client_fd, Socket& socket, const string msg){
	int send_bytes = socket.Send(client_fd, resp);
	if(send_bytes == -1) {
		cerr << "Error al enviar datos: " << strerror(errno) << endl;
		// Cerramos los sockets
		socket.Close(client_fd);
		exit(1);
	}
}

string recv_msg(const int client_fd, Socket& socket){
	string buffer;
	// Recibimos el mensaje del cliente (su puja)
	int rcv_bytes = socket.Recv(client_fd, buffer, MESSAGE_SIZE);
	if(rcv_bytes == -1) {
		cerr << "Error al recibir datos: " << strerror(errno) << endl;
		socket.Close(client_fd);
	}
	return buffer;
}
