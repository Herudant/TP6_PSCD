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
MULTISERVER=ServidorMulticliente
VUELO=vuelo

# #################### #
# FLAGS DE COMPILACION #
# #################### #
CPPFLAGS=-I. -O2 -std=c++11 -lsockets # Flags compilacion
LDFLAGS=-pthread # Flags linkado threads
##SOCKETSFLAGS=-lsocket -lnsl # Flags linkado sockets (Solaris SunOS). Descomentar para usar en hendrix

.PHONY:all

all: ${CLIENT} ${MULTISERVER} ${VUELO}

# SOCKETS
# Compilacion libreria de Sockets
${SOCKET}.o: ${SOCKET}.hpp ${SOCKET}.cpp
	${CC} -c ${CPPFLAGS} ${SOCKET}.cpp

# VUELO
# Compilacion
${VUELO}.o: ${VUELO}.hpp ${VUELO}.cpp
	${CC} -c ${CPPFLAGS} ${VUELO}.cpp

#-----------------------------------------------------------
# CLIENTE
# Compilacion
${CLIENT}.o: ${CLIENT}.cpp
	${CC} -c ${CPPFLAGS} ${CLIENT}.cpp

# Linkado
${CLIENT}: ${SOCKET}.o ${CLIENT}.o
	${CC} ${LDFLAGS} ${SOCKET}.o ${CLIENT}.o -o ${CLIENT} ${SOCKETSFLAGS}
#-----------------------------------------------------------
# MULTISERVIDOR
# Compilacion
${MULTISERVER}.o: ${MULTISERVER}.cpp
	${CC} -c ${CPPFLAGS} ${MULTISERVER}.cpp

# Linkado
${MULTISERVER}: ${SOCKET}.o ${MULTISERVER}.o ${VUELO}.o
	${CC} ${LDFLAGS} ${SOCKET}.o ${MULTISERVER}.o ${VUELO}.o -o ${MULTISERVER} ${SOCKETSFLAGS}
#-----------------------------------------------------------
# SERVIDOR
# Compilacion
${SERVER}.o: ${SERVER}.cpp
	${CC} -c ${CPPFLAGS} ${SERVER}.cpp

# Linkado
${SERVER}: ${SOCKET}.o ${SERVER}.o ${VUELO}.o
	${CC} ${LDFLAGS} ${SOCKET}.o ${SERVER}.o ${VUELO}.o -o ${SERVER} ${SOCKETSFLAGS}
#-----------------------------------------------------------

# LIMPIEZA
clean:
	$(RM) ${SOCKET}.o
	$(RM) ${CLIENT} ${CLIENT}.o
	$(RM) ${SERVER} ${SERVER}.o
	$(RM) ${CLIENT} ${CLIENT}.o
	$(RM) ${VUELO}  ${VUELO}.o
	$(RM) ${MULTISERVER} ${MULTISERVER}.o
