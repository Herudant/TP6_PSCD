#! /bin/bash
if [ "$#" -ne 3 ]; then
    echo "Se esperaba ./lanza_cliente_manual <IP> <PUERTO> <URL>"
fi

./bin/Cliente $1 $2 $3
