//*****************************************************************
// File:   Cliente.cpp
// Author:
// Date:
//*****************************************************************

/*---------------  Librerias y ficheros --------------------------------------*/
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <stdio.h>
#include "Socket.hpp"
#include <ctype.h>
#include <signal.h>

using namespace std;
/*----------------------------------------------------------------------------*/

/*---------------  Funciones privadas ----------------------------------------*/
// Envía el mensaje al cliente asociado al socket
void send_msg(const int client_fd, Socket& socket, const string msg);

// Recibe y devuelve el mensaje del cliente asociado al socket
string recv_msg(const int client_fd, Socket& socket);

// Captura señal de interrupcion para evitar cerrar el servidor
void handler(int n);

// Separa una cadena en subcadenadas cada vez que encuentra caracter separador
vector<string> decodificar(string mensaje, const char separador);

// Devuelve true si la entrada es numerica
bool isNumeric(const string& input);

// Lee de stdin y devuelve mensaje de fin o puja
string getLine_puja();
/*----------------------------------------------------------------------------*/

/*--------------- Variables globales del sistema  ----------------------------*/
const int MESSAGE_SIZE = 4001; //mensajes de no más 4000 caracteres
const string MENS_FIN("EOS");  			// End of Service
const string MENS_FIN_PUJA("EOB");  // End of Bid
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
	if(argc < 4){
		cout << "Error: Se esperaba ./Cliente <IP> <Puerto> <URL> [<auto>]\n";
		exit(1);
	}

  // Dirección y número donde escucha el proceso servidor
  string SERVER_ADDRESS = argv[1];
  int SERVER_PORT = atoi(argv[2]);
	string URL = argv[3];
  bool AUTO = false;
  if(argc == 5 && strcmp(argv[4], "auto") == 0){
    AUTO = true;
  }


	// Creación del socket con el que se llevará a cabo
	// la comunicación con el servidor.
	Socket socket(SERVER_ADDRESS, SERVER_PORT);

  // Conectamos con el servidor. Probamos varias conexiones
	const int MAX_ATTEMPS = 10;
	int count = 0;
	int socket_fd;
	do {
		// Conexión con el servidor
  	socket_fd = socket.Connect();
  	count++;

  	// Si error --> esperamos 1 segundo para reconectar
  	if(socket_fd == -1){
  	    this_thread::sleep_for(chrono::seconds(1));
  	}

  } while(socket_fd == -1 && count < MAX_ATTEMPS);

  // Chequeamos si se ha realizado la conexión
  if(socket_fd == -1){
  	return socket_fd;
  }

  // capturo señal de cierre para evitar que se termine sin cerrar los sockets.
  signal(SIGINT, handler);


  bool fin, out;
	int num_subastas = 0; //subastas en las que ha participado el cliente

  // Hacemos las peticiones
	do{
		bool ganador = false;
		string buffer;
		string mensaje;
		cout << "Esperando a que se inicie una puja ..." << endl;
		//esperar a recibir la señal de inicio de una puja y su precio
		buffer = recv_msg(socket_fd, socket);
		auto respuesta = decodificar(buffer, '#');

		cout << "\n--------------------------------------------------------------\n"
		     << "||| Puja activa ||| ==> Precio de subasta: " << respuesta.at(0)
		     << "  --- Tiempo subastado: " << respuesta.at(1) << endl
		     << "Escribir '" << MENS_FIN_PUJA << "'' para no participar\n"
		     << "\n--------------------------------------------------------------\n";

		out = false;
		fin = false;
		while(!out){
			//se envia una puja
			if (num_subastas == 3) {
				mensaje = (AUTO) ? (MENS_FIN) : getLine_puja();
			}
			else {
				mensaje = (AUTO) ? (to_string(rand() % 200 + 5)) : getLine_puja();
			}
			send_msg(socket_fd, socket, mensaje);

			if(mensaje == MENS_FIN_PUJA){
				out = true;
			}
			else if ( mensaje == MENS_FIN){
				out = true;
				fin = true;
			}
			else {
				//si no se ha pasado de la subasta, se recibe respuesta y analiza
				buffer = recv_msg(socket_fd, socket);
				respuesta = decodificar(buffer, '#');

				string msg_code = respuesta.at(0);
				string precio;
				if(respuesta.size() >= 2){
					precio = respuesta.at(1);
				}

				//si se ha cerrado ya la subasta
				if (msg_code == "SUBASTA_CERRADA"){
					out = true;
					cout << "Subasta cerrada\n";
				}
				//si hno se ha superado la puja maxima
				else if (msg_code == "PERDEDOR"){
					cout << "Su puja no ha superado la puja actual mas alta\n"
					     << "Precio actual de la subasta: " << precio << endl;

					if(AUTO){
						int tiempo_subasta = rand() % 3 + 4;
						this_thread::sleep_for(chrono::milliseconds(tiempo_subasta*1000));
					}
				}
				//se ha superado la puja maxima, por lo que eres el ganador hasta nuevo
				// aviso
				else if (msg_code == "GANADOR"){
					cout << "Por el momento es usted la puja mas alta\n"
					     << "Esperando mas notificaciones del servior...\n";

					buffer = recv_msg(socket_fd, socket);
					respuesta = decodificar(buffer, '#');

					msg_code = respuesta.at(0);
					if(respuesta.size() >= 2){
						precio = respuesta.at(1);
					}

					//eres el ganador definitivo
					if (msg_code == "FIN_GANADOR"){
						cout << "Enhorabuena! Es usted el ganador de la subasta.\n"
						     << "Enviando URL de imagen al servidor...\n"
								 << "\n-----------------------------------------------------\n";
						send_msg(socket_fd, socket, URL);
						ganador = true;
						out = true;
					}
					//tu puja ha sido superada
					else if (msg_code == "NUEVO_GANADOR"){
						cout << "Su puja ha sido superada por otro participante\n"
						     << "Precio actual de la subasta: " << precio << endl;
						if(AUTO){
							int tiempo_subasta = rand() % 20 + 25;
							this_thread::sleep_for(chrono::milliseconds(tiempo_subasta*1000));
						}
					}
					else{
						out = true;
					}
				}
			}
		}
		// se informa del fin de subasta
		if (!ganador){
			cout << "Lo sentimos, ha perdido la subasta, suerte en la siguiente\n";
		}
		++num_subastas;
	} while(!fin);

  // Cerramos el socket
  int error_code = socket.Close(socket_fd);
  if(error_code == -1){
		cerr << "Error cerrando el socket: " << strerror(errno) << endl;
  }

  cout << "ByeBye!\n";
  return error_code;
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// FUNCIONES PRIVADAS /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

vector<string> decodificar(string mensaje, const char separador){
  vector<string> ret;
  string msg = "";
  for(char ch: mensaje){
    if(ch == separador){
      ret.push_back(msg);
			msg = "";
		}
    else
      msg+=ch;
  }
	ret.push_back(msg);
  return ret;
}

string getLine_puja(){
	string ret;
	int n;
	bool correcto = false;
	 while (!correcto){
		cout << "Puja... > ";
		getline(cin, ret);
		if (ret != MENS_FIN && ret != MENS_FIN_PUJA){
			n = atoi(ret.c_str());
			if (n>0)
				correcto = true;
		}
		else
			correcto = true;
		if (!correcto){
			cout << "Puja no válida, por favor introduzca:\n"
					 <<"\t" << MENS_FIN << " para finalizar el programa\n"
					 <<"\t" << MENS_FIN_PUJA << " para no pujar y esperar otra subasta\n"
					 <<"\tO un numero entero para pujar\n";
		}
	}
	return ret;
}

// socket.send
void send_msg(const int socket_fd, Socket& socket, const string msg)
{
	int send_bytes = socket.Send(socket_fd, msg);
	if(send_bytes == -1) {
		cerr << "Error al enviar datos: " << strerror(errno) << endl;
		// Cerramos los sockets
		socket.Close(socket_fd);
		exit(1);
	}
}

// socket.recv
string recv_msg(const int socket_fd, Socket& socket)
{
	string buffer;
	// Recibimos el mensaje del servidor
	int rcv_bytes = socket.Recv(socket_fd, buffer, MESSAGE_SIZE);
	if(rcv_bytes == -1) {
		cerr << "Error al recibir datos: " << strerror(errno) << endl;
		socket.Close(socket_fd);
	}
	return buffer;
}


void handler(int n)
{
	signal(SIGINT, handler);
	cout << "Para salir escribe 'END OF SERVICE'\n";
}

bool isNumeric(const string& input) {
	bool ret = true;
	for(char ch : input){
		if(!isdigit( ch + '0' )){
			ret = false;
			break;
		}
	}
	return ret;
	//return std::all_of(input.begin(), input.end(), ::isdigit);
}
