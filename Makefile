#*****************************************************************
# File:   Makefile
# Author:
# Date:
# Coms:
#				Los ficheros fuente deben estar en un subdirectorio "src"
# 			Los resultados de la compilación se guardan en un subdirectorio "bin"
#*****************************************************************

# ######## #
# COMANDOS #
# ######## #
RM = rm -f # Comando de limpieza
CC = g++ # Comando de compilacion

###########################
#    VARIABLES GLOBALES   #
###########################

SRC=src
BIN=bin
IMGS=imgs
CLIENT=Cliente
SERVER=Servidor
SOURCES=$(wildcard $(SRC)/*.cpp)									#añade todos los nombres que terminan en .cpp
SOURCES:=$(filter-out $(SRC)/$(CLIENT).cpp, $(SOURCES))				#excluye cliente
SOURCES:=$(filter-out $(SRC)/$(SERVER).cpp, $(SOURCES))				#excluye el servidor
OBJECTS=$(patsubst %,$(BIN)/%,$(notdir $(SOURCES:.cpp=.o)))		#cambia nombres de .cpp a .o cambiando también el directorio
TARGET_1=$(BIN)/$(SERVER)
TARGET_2=$(BIN)/$(CLIENTE)

# #################### #
# FLAGS DE COMPILACION #
# #################### #


# -fmax-errors=1
CPPFLAGS=-I. -I/usr/local/include -O2 -std=c++11 -Werror
LDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib -lm -pthread -lcurl -lX11
#SOCKETSFLAGS=-lsocket -lnsl # Flags linkado sockets (Solaris SunOS)

.PHONY: all cliente servidor
all: folder cliente servidor

# #################### #
#       CLIENTE 	     #
# #################### #
cliente: $(BIN)/$(CLIENT)

$(BIN)/$(CLIENT): $(OBJECTS) $(BIN)/$(CLIENT).o
	$(CC) $(LDFLAGS) $(OBJECTS) $(BIN)/$(CLIENT).o -o $@
#${SOCKETSFLAGS} #descomentar para Hendrix

$(BIN)/$(CLIENT).o: $(SRC)/$(CLIENT).cpp
	$(CC) $(CPPFLAGS) $< -c -o $@

# #################### #
# 		SERVIDOR 	       #
# #################### #
servidor: $(BIN)/$(SERVER)

$(BIN)/$(SERVER): $(OBJECTS) $(BIN)/$(SERVER).o
	$(CC) $(LDFLAGS) $(OBJECTS) $(BIN)/$(SERVER).o -o $@
#${SOCKETSFLAGS} #descomentar para Hendrix

$(BIN)/$(SERVER).o: $(SRC)/$(SERVER).cpp
	$(CC) $(CPPFLAGS) $< -c -o $@

$(BIN)/%.o: $(SRC)/%.cpp			#compilar excepto servidor y cliente
	$(CC) -c $(CPPFLAGS) $< -o $@

echo:
	@echo "obj: $(OBJECTS)"
	@echo "src: $(SOURCES)"


#Crear la carpeta
folder:
	@mkdir -p $(BIN)
	@mkdir -p $(IMGS)

#limpieza
clean:
	$(RM) $(OBJECTS)
	$(RM) $(BIN)/$(CLIENT).o $(BIN)/$(CLIENT)
	$(RM) $(BIN)/$(SERVER).o $(BIN)/$(SERVER)
	rm -rf $(BIN)
