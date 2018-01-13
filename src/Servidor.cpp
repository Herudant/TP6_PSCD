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
#include <fstream>
#include "subasta.hpp"			// Ficheros
#include "valla.hpp"
#include "Socket.hpp"
#include "CImg.hpp"
#include "ImageDownloader.hpp"

#define VERBOSE

using namespace std;
/*----------------------------------------------------------------------------*/

/*---------------  Funciones privadas ----------------------------------------*/
// Atiende peticion Cliente
void dispatcher(int client_fd, Socket& socket, Subasta& subasta,
								Valla& valla, const int id);

// Gestor de las vallas publicitarias
void gestor_valla(Valla& valla);

// Gestor de subastas, crea subastas que duran un periodo de tiempo aleatorio
void subastador(Subasta& subasta);

void administrador(int fd, Socket& socket, Subasta& subasta, Valla& valla);


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

atomic_bool FIN_SERVICIO = ATOMIC_VAR_INIT(false);	// indica la terminación del servicio
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	#ifdef VERBOSE
	cout << "EMPEZANDO\n";
	#endif
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
	#ifdef VERBOSE
	cout << "BIN HECHO\n";
	#endif
	// Capturamoss la señal para evitar fallos
	signal(SIGINT, handler);

	// Descargamos la imagen por defecto
	char ruta[100] = "imgs/default.jpg";
	char cURL[500] = "https://upload.wikimedia.org/wikipedia/commons/thumb/5/5b/Insert_image_here.svg/1280px-Insert_image_here.svg.png";

	ImageDownloader downloader;
	downloader.downloadImage(cURL, ruta);
	#ifdef VERBOSE
	cout << "IMAGEN DESCARGADA\n";
	#endif

	/*--------------- Lanzamos modulos del sistema -----------------------------*/
	Subasta subasta;
	Valla valla;

	// Lanzamos el thread de valla
	thread t_valla(&gestor_valla, ref(valla));
	t_valla.detach();
	#ifdef VERBOSE
	cout << "GESTOR_VALLA LANZADO\n";
	#endif
	// Lanzar thread subasta
	thread t_subastador(&subastador, ref(subasta));
	t_subastador.detach();
	#ifdef VERBOSE
	cout << "SUBASTADOR LANZADO\n";
	#endif
	// Lanzar thread administrador
	thread t_administrador(&administrador, socket_fd, ref(socket),
	 											ref(subasta), ref(valla));
	t_administrador.detach();
	#ifdef VERBOSE
	cout << "ADMINISTRADOR LANZADO\n";
	#endif
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
void dispatcher(int client_fd, Socket& socket, Subasta& subasta,
								Valla& valla, const int id)
{
	int error_code;
	const string MENS_FIN("END OF SERVICE");
	const string MENS_FIN_PUJA("PASAR SUBASTA");

	// Buffer para recibir el mensaje
	string buffer;

	bool out, fin_cliente = false; // Inicialmente no salir del bucle
	int num_subasta = subasta.getNum_subastas();

	// Mientras dura el servicio de subastas
	while(!FIN_SERVICIO || !fin_cliente){

		string msg;
		// Esperamos a que se inicie una subasta
		int ultimo_precio, tiempo_subasta;
		ultimo_precio = subasta.entrarSubasta(num_subasta);
		tiempo_subasta = subasta.getTiempo_subasta();
		msg = to_string(ultimo_precio) + "#" + to_string(tiempo_subasta);
		//notificar al cliente de la subasta
		send_msg(client_fd, ref(socket), msg);

		out = false;
		while(!out) {

			// Recibimos el mensaje del cliente (su puja)
			buffer = recv_msg(client_fd, ref(socket));
			cout << "Mensaje recibido: " << id << " -> " << buffer << "\n";
			if(buffer == MENS_FIN_PUJA){
				out = true; // Salir del bucle
				++num_subasta;
			}
			else if (buffer == MENS_FIN){
				out = true;
				fin_cliente = true;		// finalizar servicio del cliente
			}
			else {
				// Cliente hace puja, si devuelve -1 soy ganador
				int precio_ganador = subasta.pujar(id, atoi(buffer.c_str()));

				int tiempo_subasta = subasta.getTiempo_subasta();
				// Enviamos la respuesta
				string resp;
				if (!subasta.getActiva() || subasta.getNum_subastas() != num_subasta) {
					#ifdef VERBOSE
					cout << "subastas: "<< subasta.getNum_subastas()
							 << "y el cliente esta en subasta: " << num_subasta
							 << ", ademas la subasta activa? " << subasta.getActiva() << endl;
					#endif
					resp = "SUBASTA_CERRADA#-1";
					send_msg(client_fd, ref(socket), resp);
					num_subasta = subasta.getNum_subastas();
					#ifdef VERBOSE
					cout << "realojado en la puja " << num_subasta << endl;
					#endif
					out = true;
				}
				else {
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
							valla.addPeticion(buffer, tiempo_subasta);
							#ifdef VERBOSE
								cout << "Peticion añadida: " << buffer << ","
								     << tiempo_subasta << endl;
							#endif
							subasta.avisarSubastador();
							out = true;
							++num_subasta;
						}
					}
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
	string URL, msg;

	char ruta[100] = "imgs/image.jpg";
	char cURL[400] = "";

  #ifdef VERBOSE
	cout << "CREANDO VALLA 1\n";
  #endif
	// VALLA_1
	cimg_library::CImg<unsigned char> img_("imgs/default.jpg");
	cimg_library::CImgDisplay valla_0(img_.resize(_WIDTH, _HEIGHT),"VALLA 0");
	valla_0.resize(_WIDTH, _HEIGHT);
	valla_0.move(0, 0); // Esquina superior izquierda

  #ifdef VERBOSE
	cout << "CREANDO VALLA 2\n";
  #endif

	// VALLA_2
	cimg_library::CImgDisplay valla_1(img_.resize(_WIDTH, _HEIGHT),"VALLA 1");
	valla_1.resize(_WIDTH, _HEIGHT);
	valla_1.move(0, 850); // Esquina superior izquierda

  #ifdef VERBOSE
	cout << "VALLAS CREADAS, ATENDIENDO PETICIONES\n";
  #endif

	while (1) {
		// Atiende petición, recibe {n_valla, URL, tiempo}
		tie(n_valla, URL, tiempo) = valla.atenderPeticion();

		#ifdef VERBOSE
		cout << "\n\t----------------------------------------------------\n"
		     << "\tMOSTRANDO VENTANA (" << to_string(n_valla) << "): "
				 << to_string(tiempo) << " segundos, " << URL
		     << "\n\t----------------------------------------------------\n";
	  #endif

		//Descargamos imagen
		strcpy(cURL, URL.c_str());
		downloader.downloadImage(cURL, ruta);
		//valla.write(msg, fs);

		if (n_valla == 0) {
			#ifdef VERBOSE
				cout << "Modificando valla 0\n";
			#endif
			printImage(ruta, tiempo, valla_0);

			//Avisamos de la finalizacion y mostramos valla por defecto
			printImage("imgs/default.jpg", 0, valla_0);
			valla.finPeticion(tiempo, n_valla);
		} else{
			#ifdef VERBOSE
				cout << "Modificando valla 1\n";
			#endif
			printImage(ruta, tiempo, valla_1);

			//Avisamos de la finalizacion y mostramos valla por defecto
			printImage("imgs/default.jpg", 0, valla_1);
			valla.finPeticion(tiempo, n_valla);
		}

	}

}

// Imprime una imagen en una ventana durante un tiempo
void printImage(const string ruta, time_t tiempo, cimg_library::CImgDisplay& v)
{
	char rutaIMG[100];
	strcpy(rutaIMG, ruta.c_str());

	cimg_library::CImg<unsigned char> img_sec(rutaIMG);
	v.render(img_sec.resize(_WIDTH, _HEIGHT));
	v.paint(); // Repintar nueva imagen en la valla
	if(tiempo > 0)
		this_thread::sleep_for(chrono::milliseconds(tiempo*1000));
}


void administrador(int socketfd, Socket& socket, Subasta& subasta, Valla& valla)
{
	string mensaje;
	time_t tiempo_total, tiempo_contratado, tiempo_imagenes;
	int num_peticiones, num_imagenes;
	while(1){
		getline(cin,mensaje);
		if (mensaje == "END OF SERVICE"){
			#ifdef VERBOSE
				cout << "END OF SERVICE DETECTADO\n";
			#endif
			FIN_SERVICIO = true;

			// Iniciar la terminación ordenada del servicio
			cout << "Esperando a la finalización de las subastas....\n";
			subasta.cerrarServicio();
			cout << "Subasta finalizada\n";

			cout << "Esperando a la finalización de las peticiones restantes....\n";
			valla.cerrarServicio();
			cout << "Peticiones finalizadas\n";

		  cout << "Cerrando socket....\n";
			int error_code = socket.Close(socketfd);
			if(error_code == -1)
				cerr << "Error cerrando el socket: " << strerror(errno) << endl;

			cout << "Bye bye" << endl;
			exit(1);

			break;
		}
		else if (mensaje =="PRINT HISTORICA"){
			// Mostrar información histórica del sistema (num imagenes y tiempo)
			tiempo_imagenes = valla.getTiempo_imagenes_mostradas();
			num_imagenes = valla.getNum_imagenes();
		  cout << "------ INFORMACION HISTORICA DEL SISTEMA -------------------------"
					 << "\n\tNumero de imagenes mostradas: " << to_string(num_imagenes)
					 <<	"\n\tTiempo de imagenes mostradas: " << to_string(tiempo_imagenes)
			     << "\n---------------------------------------------------------------\n";
			//valla.write(msg, ref(fs));

		}
		else if (mensaje == "PRINT ESTADO"){
			// Mostrar información del estado del sistema (num peticiones y tiempo contratado)
			num_peticiones = valla.getNum_peticiones();
			tiempo_contratado = valla.getTiempo_estimado();
			tiempo_total = valla.getTiempo_total();
			cout << "------ INFORMACION DEL ESTADO DEL SISTEMA ------------------------"
					 << "\n\tNumero de peticiones : " << to_string(num_peticiones)
					 <<"\n\tTiempo contrado      : " << to_string(tiempo_contratado)
					 <<"\n\tTiempo total         : " << to_string(tiempo_total)
					 <<	"\n---------------------------------------------------------------\n";
			//valla.write(msg, ref(fs));
		}
		else {
			cout << "ERROR: LAS PETICIONES DICPONIBLES SON LAS SIGUIENTES:\n"
					 << "\t1. PRINT HISTORICA\n"
					 << "\t2. PRINT ESTADO\n"
					 << "\t3. END OF SERVICE\n";
		}
	}


}

// Gestor de subastas, crea subastas que duran un periodo de tiempo aleatorio
void subastador(Subasta& subasta)
{
	srand(time(NULL));
	while(!FIN_SERVICIO || !subasta.maxSubastas(MAX_SUBASTAS)){
		int precio_subasta = rand() % 200 + 5;
		int tiempo_subasta = rand() % 20  + 2;
		#ifdef VERBOSE
			cout << "INICIANDO SUBASTA: " << precio_subasta << "€, "
				   << tiempo_subasta << " segundos.\n";
		#endif

		subasta.iniciarSubasta(precio_subasta, tiempo_subasta);

		int duracion_subasta = rand() % 20 + 10;
		this_thread::sleep_for(chrono::milliseconds(duracion_subasta *1000));

		#ifdef VERBOSE
			cout << "CERRANDO SUBASTA...\n";
		#endif
		subasta.cerrarSubasta();
		#ifdef VERBOSE
			cout << "SUBASTA CERRADA\n";
		#endif
	}
}

// socket.send
void send_msg(const int client_fd, Socket& socket, const string msg)
{
	int send_bytes = socket.Send(client_fd, msg);
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
