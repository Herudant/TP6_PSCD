#! /bin/bash

if [ "$#" -ne 1 ]; then
    echo "Se esperaba ./lanza_servidor <PUERTO>"
fi

cd bin

./bin/Servidor $1
