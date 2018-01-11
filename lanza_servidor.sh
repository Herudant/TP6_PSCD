#! /bin/bash

if [ "$#" -ne 1 ]; then
    echo "Se esperaba ./lanza_servidor <PUERTO>"
fi

cd bin

./Servidor $1
