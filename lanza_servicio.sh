#! /bin/bash
if [ "$#" -ne 1 ]; then
    echo "Se esperaba ./lanza_servicio <PUERTO>"
    exit 1
fi

./bin/Servidor $1
