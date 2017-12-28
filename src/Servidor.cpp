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

#include "vallaPrincipal.h"
#include "vallaSecundaria.h"

#include "Socket.h"
#include "CImg.h"
#include "ImageDownloader.h"

using namespace std;



const int MESSAGE_SIZE = 4001; //mensajes de no más 4000 caracteres

const int PRINCIPAL_WIDTH = 800;
const int PRINCIPAL_HEIGHT = 800;
const int SECUNDARIA_WIDTH = 800;
const int SECUNDARIA_HEIGHT = 400;

const int NUMVALLASEC = 2;
VallaSecundaria serv_secundario;
VallaPrincipal serv_principal;


void avisarFin(int socket_fd, Socket& socket);
void imprImg(const string ruta, time_t tiempo,cimg_library::CImgDisplay& valla);
void vallaPral();
void vallaSec(int n);
void crearImg(const string ruta, const int numVent,cimg_library::CImgDisplay& vallasec);
void dispatcher(int client_fd, Socket& socket);
void handler(int n){
	signal(SIGINT, handler);
	cout << "Para salir escribe 'END OF SERVICE' \n";
}

//-------------------------------------------------------------
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

	// Lanzamos los threads de las vallas
	thread vallaP(&vallaPral);
	vallaP.detach();

	thread vallaS[NUMVALLASEC];
	for (int i = 0; i < NUMVALLASEC; i++) {
		vallaS[i] = thread(&vallaSec, i + 1);
		vallaS[i].detach();
	}

	// Lanzamos el thread para avisar de fin
	thread p(&avisarFin,socket_fd, ref(socket));
	p.detach();


	// Listen
	int max_connections = 2;
	int error_code = socket.Listen(max_connections);
	if(error_code == -1) {
		cerr << "Error en el listen: " << strerror(errno) << endl;
		// Cerramos el socket
		socket.Close(socket_fd);
		exit(1);
	}

	// Bucle de aceptacion de peticiones
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
		thread t(&dispatcher, client_fd, ref(socket));
		t.detach();
	}

	return 0;
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


// Imprime en una ventana durante un tiempo la ruta de imagen dada
void imprImg(const string ruta, time_t tiempo, cimg_library::CImgDisplay& valla) {
	char rutaIMG[100];
	strcpy(rutaIMG, ruta.c_str());
	cimg_library::CImg<unsigned char> img_sec(rutaIMG);
	valla.render(img_sec.resize(SECUNDARIA_WIDTH, SECUNDARIA_HEIGHT));
	valla.paint(); // Repintar nueva imagen en la valla
	if(tiempo > 0)
		this_thread::sleep_for(chrono::milliseconds(tiempo*1000));
}

// Thread que gestiona la ventana principal
void vallaPral() {

	int tiempo;
	string URL;
	ImageDownloader downloader;
	char ruta[100] = "imgs/imagePral.jpg";
	char cURL[500] = "https://upload.wikimedia.org/wikipedia/commons/thumb/5/5b/Insert_image_here.svg/1280px-Insert_image_here.svg.png";

	// VALLA PRINCIPAL
	cimg_library::CImg<unsigned char> img_principal("imgs/default.jpg");
	cimg_library::CImgDisplay valla_principal(img_principal.resize(PRINCIPAL_WIDTH, SECUNDARIA_WIDTH),"VALLA PRINCIPAL");
	valla_principal.resize(PRINCIPAL_WIDTH, SECUNDARIA_WIDTH);
	valla_principal.move(0, 0); // Esquina superior izquierda


	while (1) {

		//Datos imagen a mostrar
		serv_principal.datosImagen(URL, tiempo);

		//Descargamos imagen
		strcpy(cURL, URL.c_str());
		downloader.downloadImage(cURL, ruta);


		cout << "\n\t\t ---------------------------------------\n";
		cout << "\t\t MOSTRANDO VENTANA PRINCIPAL: "  << to_string(tiempo) << " segundos, " << URL << endl;
		cout << "\t\t ---------------------------------------\n";
		imprImg(ruta, tiempo, valla_principal);
		imprImg("imgs/default.jpg", 0, valla_principal);

		//Avisamos de la finalizacion
		serv_principal.avisar(tiempo);

	}

}

void vallaSec(int n) {

	int tiempo;
	string URL;
	char cURL[500] = "https://upload.wikimedia.org/wikipedia/commons/thumb/5/5b/Insert_image_here.svg/1280px-Insert_image_here.svg.png";
	char ruta[100];
	ImageDownloader downloader;

	if ( n == 1)
		strcpy(ruta,"imgs/imageSec1.jpg");
	else if ( n == 2)
		strcpy(ruta,"imgs/imageSec2.jpg");


	cimg_library::CImgDisplay valla_sec;

	crearImg("imgs/default.jpg", n, valla_sec);


	while (1) {

		// Pedimos datos de la imagen a mostrar.
		serv_secundario.datosImagen(URL, tiempo, n);


		//Descargamos imagen
		strcpy(cURL, URL.c_str());
		downloader.downloadImage(cURL, ruta);

		cout << "\n------ -----------------------------------------\n";
		cout << "\t\tMOSTRANDO VENTANA SECUNDARIA (" << n << "): "  << to_string(tiempo) << " segundos, " << URL << endl;
		cout << "-------- ---------------------------------------\n";

		imprImg(ruta, tiempo, valla_sec);

		imprImg("imgs/default.jpg", 0, valla_sec);
		serv_secundario.avisar(tiempo, n);
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

void dispatcher(int client_fd, Socket& socket){
	int error_code;
	char MENS_FIN[]="END OF SERVICE";


	// Buffer para recibir el mensaje
	int length = 100;
	string buffer;

	bool out = false; // Inicialmente no salir del bucle
	while(!out) {
		// Recibimos el mensaje del cliente: (  tipo de valla ; tiempo ; URL; )
		int rcv_bytes = socket.Recv(client_fd, buffer, MESSAGE_SIZE);
		if(rcv_bytes == -1) {
			cerr << "Error al recibir datos: " << strerror(errno) << endl;
			socket.Close(client_fd);
		}


		// Si recibimos "END OF SERVICE" --> Fin de la comunicación
		if(buffer == MENS_FIN)
			out = true; // Salir del bucle
		else {

			cout << "Mensaje recibido por: " << client_fd << " -> '" << buffer << "'" << endl;

			string cadAux, valla, url;
			int tiempo;
			int parametro=0;
			int len = buffer.size();
			cadAux = "";
			for (int i = 0; i < len ; ++i) {
				if( buffer[i] != ';' )
					cadAux = cadAux + buffer[i];
				else {
					switch (parametro){
						case 0:
							//cout << "Asignando valla: " << cadAux << endl;
							valla = cadAux;
							cadAux = "";
							parametro++;
							break;
						case 1:

							tiempo = stoi(cadAux);
							//cout << "Asignando tiempo: " << tiempo << endl;
							cadAux = "";
							parametro++;
							break;
						case 2:
							//cout << "Asignando url: " << cadAux << endl;
							url = cadAux;
							cadAux = "";
							break;
					}
				}

			}

			int coste = 0;
			time_t horaVisualizacion;
			struct tm * timeinfo;

			if ( valla == "principal")
				serv_principal.solicitar(url, tiempo,coste, horaVisualizacion);
			else if ( valla == "secundaria")
				serv_secundario.solicitar(url, tiempo,coste, horaVisualizacion);



			// Enviamos la respuesta
			string resp;

			if( coste > 0 ){
				timeinfo = localtime(&horaVisualizacion);
				resp = "precio: " + to_string(coste) + "; hora visualizacion: " + asctime(timeinfo);
			}
			else
				resp = "servidor lleno, intentelo mas tarde";

			int send_bytes = socket.Send(client_fd, resp);
			if(send_bytes == -1) {
				cerr << "Error al enviar datos: " << strerror(errno) << endl;
				// Cerramos los sockets
				socket.Close(client_fd);
				exit(1);
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


