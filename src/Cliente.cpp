//*****************************************************************
// File:   Cliente.cpp
// Author:
// Date:
//*****************************************************************

#include <iostream>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <string>
#include "Socket.hpp"
#include <ctype.h>
#include <signal.h>
#include <vector>

using namespace std;
/*---------------  Funciones privadas ----------------------------------------*/

// Envía el mensaje al cliente asociado al socket
void send_msg(const int client_fd, Socket& socket, const string msg);

// Recibe y devuelve el mensaje del cliente asociado al socket
string recv_msg(const int client_fd, Socket& socket);

// Captura señal de interrupcion para evitar cerrar el servidor
void handler(int n);

vector<string> decodificar(string mensaje, const char separador);

bool isNumeric(const string& input);


string getLine_puja();

/*----------------------------------------------------------------------------*/

const int MESSAGE_SIZE = 4001; //mensajes de no más 4000 caracteres

int main(int argc, char *argv[]) {

	if(argc != 3){
		cout << "Error: Se esperaba ./Cliente <IP> <Puerto>\n";
		exit(1);
	}

	const string MENS_FIN("END OF SERVICE");
	const string MENS_FIN_PUJA("PASAR SUBASTA");
  // Dirección y número donde escucha el proceso servidor
  string SERVER_ADDRESS = argv[1];
  int SERVER_PORT = atoi(argv[2]);

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

  // Hacemos las peticiones
	do{
		bool ganador = false;
		string buffer;
		string mensaje;
		cout << "Esperando a que se inicie una puja ..." << endl;
		//esperar a recibir la señal de inivio de una puja y su precio
		buffer = recv_msg(socket_fd, socket);
		cout << "Puja activa con precio de subasta: " << buffer << endl;
		cout << "Escribir 'PASAR SUBASTA' para no participar\n";

		mensaje = getLine_puja();

		out = false;
		while(!out){
			//se envia una puja
			send_msg(socket_fd, socket, puja);

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
				auto respuesta = decodificar(buffer, '#');

				string msg_code = respuesta.at(0);
				string precio = respuesta.at(1);

				//si se ha cerrado ya la subasta
				if (msg_code == "SUBASTA_CERRADA"){
					out = true;
				}
				//si hno se ha superado la puja maxima
				else if (msg_code == "PERDEDOR") {
					cout << "Su puja no ha superado la puja actual mas alta\n";
					cout << "Precio actual de la subasta: " << precio << endl;
					cout << "Quiere volver a pujar? ('PASAR SUBASTA' para salir)\n"
					mensaje = getLine_puja();
				}
				//se ha superado la puja maxima, por lo que eres el ganador hasta nuevo
				// aviso
				else if (msg_code == "GANADOR"){
					cout << "Por el momento es usted la puja mas alta\n"
					     << "Esperando mas notificaciones del servior...\n";

					buffer = recv_msg(socket_fd, socket);
					respuesta = decodificar(buffer, '#');

					msg_code = respuesta.at(0);
					precio = respuesta.at(1);

					//eres el ganador definitivo
					if (msg_code == "FIN_GANADOR") {
						string url;
						cout << "Enhorabuena! Es usted el ganador de la subasta,"
						     << " introduzca una URL por favor: ";
						getline(cin,url);
						send_msg(socket_fd, socket, url);
						ganador = true;
					}
					//tu puja ha sido superada
					else if (msg_code == "NUEVO_GANADOR"){
						cout << "Su puja ha sido superada por otro participante " << endl;
						cout << "Precio actual de la subasta: " << precio << endl;
						cout << "Quiere volver a pujar? ('PASAR SUBASTA' para salir)\n"
						mensaje = getLine_puja();
					}
				}
			}
		}
		// se informa del fin de subasta
		if (!ganador){
			cout << "Lo sentimos, ha perdido la subasta, suerte en la siguiente\n";
		}
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
	do{
		cout << "Puja... > ";
		getline(cin, ret);
	}while(!isNumeric(mensaje) && mensaje != MENS_FIN && mens != MENS_FIN_PUJA);
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
	cout << "Para salir escribe 'END OF SERVICE' \n";
}

bool isNumeric(const string& input) {
	return std::all_of(input.begin(), input.end(), ::isdigit);
}
