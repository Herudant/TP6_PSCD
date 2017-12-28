//*****************************************************************
// File:   Cliente.cpp
// Author: Eduardo Alonso
// Date:   diciembre 2017
// Coms:   Ejemplo de cliente con comunicación síncrona mediante sockets
//         Compilar el fichero "Makefile" asociado, mediante
//         "make".
//*****************************************************************

#include <iostream>
#include <chrono>
#include <thread>
#include <list>
#include <string>
#include <stdlib.h>     /* atoi */
#include "Socket.hpp"

using namespace std;

const int MESSAGE_SIZE = 4001; //mensajes de no más 4000 caracteres

void handler(int n){
	signal(SIGINT, handler);
	cout << "Para salir escribe 'END OF SERVICE' \n";
}

list<string> decodificar(string mensaje, const char separador){
  list<string> ret;
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

int main(int argc, char *argv[]) {

	if(argc != 3){
		cout << "Error: Se esperaba ./Cliente IP Puerto\n";
		exit(1);
	}

	const string MENS_FIN("END OF SERVICE");
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

	//signal(SIGINT, handler);
	string mensaje;

	do{
		string fila = "", asiento = "";
		mensaje = "";
		// Leer mensaje de la entrada estandar
		cout << "Introduzca fila (1-10) y asiento (1-4)  "
		     << "(para salir escriba 'END OF SERVICE') \n"
				 << "> fila: ";

		// Obtener fila
		do{
			getline(cin, mensaje);
			int msg = atoi(mensaje.c_str());
			if(msg >= 1 && msg <= 10){
				// fila introducida correctamente, pasamos a asiento
				fila = mensaje;
				cout << "> asiento: ";
				getline(cin, mensaje);
			}
			else{
				cout << "\nNúmero de fila incorrecto, introduzca un número entre 1-10\n"
				     << "> fila: ";
			}
		}while(fila == "" && mensaje != MENS_FIN);

		// Obtener asiento
		while(asiento == "" && mensaje != MENS_FIN){
			int msg = atoi(mensaje.c_str());
			if(msg >= 1 && msg <= 4){
				cout << "Realizando peticion.... \n";
				asiento = mensaje;
				mensaje = fila + "#" + asiento;
			}
			else{
				cout << "\nNúmero de asiento incorrecto, introduzca un número entre 1-4\n"
						 << "> asiento: ";
				getline(cin, mensaje);
			}
		};

		// Enviamos el mensaje, como cadena de C
		cout << "ENVIANDO MENSAJE: " << mensaje << endl;
	  int send_bytes = socket.Send(socket_fd, mensaje);

	  if(send_bytes == -1){
			cerr << "Error al enviar datos: " << strerror(errno) << endl;
			// Cerramos el socket
			socket.Close(socket_fd);
			exit(1);
		}

		if(mensaje != MENS_FIN){
		    // Buffer para almacenar la respuesta, como char[]
		    string buffer;

		    // Recibimos la respuesta del servidor
		    int read_bytes = socket.Recv(socket_fd, buffer, MESSAGE_SIZE);
				if(read_bytes > 0 ){
					list<string> respuesta = decodificar(buffer, '#');

					cout << respuesta.front();
					string tipo_respuesta = respuesta.front();
					respuesta.pop_front();
					cout << "  -- " << respuesta.front() << endl;
					if(tipo_respuesta == "RESERVADO"){
							cout << "RESERVA REALIZADA CORRECTAMENTE:    "
							     << "FILA: " << fila
									 << " - ASIENTO : " << asiento
									 << "\n----------------------------------------------- \n\n\n";
					}
					else if(tipo_respuesta == "OCUPADO") {
							cout << "PLAZA SOLICITADA OCUPADA, ASIENTOS DISPONIBLES.... \n";
							/* Cada asiento esta representado por un número de tal forma que
							 * la primera cifra del número indica la fila (0-9) y la segunda
							 * cifra indica el asiento (1-3)
							*/
							for(string asiento : respuesta){
								int aux = atoi(asiento.c_str());
								cout << "\t Fila: "    << (aux % 10) + 1
										 << " - Asiento: " << (aux / 10) + 1 << endl;
								respuesta.pop_front();
							}
							cout << "\n ----------------------------------------------- \n\n\n";
					}
					else if(tipo_respuesta == "COMPLETO") {
							cout << "VUELO COMPLETO, CERRANDO SESION DE CLIENTE\n";
							cout << "\n ----------------------------------------------- \n\n\n";
							mensaje = MENS_FIN;
					}
					else{
							cout << "ERROR EN LA RECEPCION DEL MENSAJE\n";
							cout << "\n ----------------------------------------------- \n\n\n";
					}
			}
		}
	} while(mensaje != MENS_FIN);

  // Cerramos el socket
  int error_code = socket.Close(socket_fd);
  if(error_code == -1){
	cerr << "Error cerrando el socket: " << strerror(errno) << endl;
  }

  return error_code;
}
