#! /bin/bash
if [ "$#" -ne 1 ]; then
    echo "Se esperaba ./lanza_servicio <PUERTO>"
fi

./bin/Servidor $1
