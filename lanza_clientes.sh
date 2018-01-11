#! /bin/bash

cd bin

if [ "$#" -ne 2 ]; then
    echo "Se esperaba ./lanza_clientes <IP> <PUERTO>"
fi


for i in {1..3}
do
	xterm -e ./bin/Cliente $1 $2 auto &
done
