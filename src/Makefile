#*****************************************************************
# File:   Makefile
# Author: PSCD-Unizar
# Date:   noviembre 2015
# Coms:   Genera sockets, cliente y servidor para la práctica 5
#         Ejecutar como "make"
#         Para compilar en Hendrix (Solaris OS), descomentar
#         "#${SOCKETSFLAGS}"
#*****************************************************************

# ######## #
# COMANDOS #
# ######## #
RM = rm -f # Comando de limpieza
CC = g++ # Comando de compilacion

# ######################## # 
# VARIABLES GLOBALES       #
# Definimos:               #
#   * Librería de sockets  # 
#   * Cliente              #
#   * Servidor             #
#   * ServidorMulticliente #
# ######################## #
SOCKET=Socket
CLIENT=Cliente
SERVER=Servidor
PRAL=vallaPrincipal
SEC = vallaSecundaria
imageDownloader=ImageDownloader

# #################### #
# FLAGS DE COMPILACION #
# #################### #
CPPFLAGS=-I. -O2 -std=c++11 -lsockets # Flags compilacion
LDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib -lm -pthread -lcurl -lX11  # Flags linkado threads
SOCKETSFLAGS=-lsocket -lnsl # Flags linkado sockets (Solaris SunOS)

.PHONY:all

all: ${CLIENT} ${SERVER}


	
# SOCKETS
# Compilacion libreria de Sockets
${SOCKET}.o: ${SOCKET}.h ${SOCKET}.cpp
	${CC} -c ${CPPFLAGS} ${SOCKET}.cpp
	
${PRAL}.o: ${PRAL}.h ${PRAL}.cpp 
	${CC} -c ${CPPFLAGS} ${PRAL}.cpp
	
${SEC}.o: ${SEC}.h ${SEC}.cpp
	${CC} -c ${CPPFLAGS} ${SEC}.cpp

${imageDownloader}.o: ${imageDownloader}.h ${imageDownloader}.cpp
	${CC} -c ${CPPFLAGS} ${imageDownloader}.cpp
#-----------------------------------------------------------
# CLIENTE
# Compilacion
${CLIENT}.o: ${CLIENT}.cpp
	${CC} -c ${CPPFLAGS} ${CLIENT}.cpp

# Linkado
${CLIENT}: ${SOCKET}.o ${CLIENT}.o  
	${CC} ${LDFLAGS} ${SOCKET}.o ${CLIENT}.o -o ${CLIENT} #${SOCKETSFLAGS} #descomentar para Hendrix
#-----------------------------------------------------------
# SERVIDOR
# Compilacion
${SERVER}.o: ${SERVER}.cpp 
	${CC} -c ${CPPFLAGS} ${PRAL}.o ${SERVER}.cpp

# Linkado
${SERVER}: ${SOCKET}.o  ${PRAL}.o ${SEC}.o ${imageDownloader}.o ${SERVER}.o
	${CC} ${LDFLAGS} ${SOCKET}.o ${SERVER}.o ${PRAL}.o ${SEC}.o ${imageDownloader}.o -o ${SERVER} #${SOCKETSFLAGS} #descomentar para Hendrix
#-----------------------------------------------------------


# LIMPIEZA
clean:
	$(RM) ${SOCKET}.o
	$(RM) ${CLIENT} ${CLIENT}.o
	$(RM) ${SERVER} ${SERVER}.o