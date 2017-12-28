//*****************************************************************
// File:   Cliente.cpp
// Author: PSCD-Unizar
// Date:   noviembre 2017
// Coms:   Ejemplo de cliente con comunicación síncrona mediante sockets
//         Comr el fichero "Makefile" asociado, media
//         "make". afskasfjhjasfhjashf
//*****************************************************************

#include <iostream>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <string>
#include "Socket.hpp"
#include <ctype.h>
#include <signal.h>

using namespace std;

const int MESSAGE_SIZE = 4001; //mensajes de no más 4000 caracteres

void handler(int n){
	signal(SIGINT, handler);
	cout << "Para salir escribe 'END OF SERVICE' \n";
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
			// hola pepito

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


    string valla,tiempo,url, mensaje;
    bool fin = false;

  // Hacemos las peticiones
	do{
		// PETICION#ARG1#ARG2...#ARGn
		// 1º espera recibir subasta con precio y tiempo
					// receive - SUBASTA#PRECIO#TIEMPO
		// 2º puja
				 // send - PRECIO
		// 3º espera recibir mensaje de ganador o perdedor
				// receive - GANADOR/PERDEDOR#PRECIO
				// si llega ganador esperamos a recibir otro mensaje cuando acabe la puja o cuando otro le supere
				// si llega perdedor volvemos a 1.

		mensaje = "";
		cout << "Nueva peticion (para salir escriba 'END OF SERVICE')\n";

		// Tiempo
		if (!fin){
			cout << "Tiempo: ";
			getline(cin,tiempo);

			if ( tiempo == MENS_FIN)
				fin = true;
			mensaje = mensaje + tiempo + ";";
		}


		// URL de la imagen
		if(!fin){
			cout << "URL: ";
			getline(cin,url);

			if ( url == MENS_FIN )
				fin = true;

			mensaje = mensaje + url + ";";
		}

		if (fin) mensaje = MENS_FIN;

		// Enviamos el mensaje
	    int send_bytes = socket.Send(socket_fd, mensaje);

	    if(send_bytes == -1){
			cerr << "Error al enviar datos: " << strerror(errno) << endl;
			// Cerramos el socket
			socket.Close(socket_fd);
			exit(1);
		}

		if(!fin){
			// Recibimos la respuesta del servidor y la mostramos
		    string buffer;
		    int read_bytes = socket.Recv(socket_fd, buffer, MESSAGE_SIZE);
		    cout << "Mensaje recibido: '" << buffer << "\n---------------\n";
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
