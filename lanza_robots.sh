#! /bin/bash
if [ "$#" -ne 2 ]; then
    echo "Se esperaba ./lanza_robots <IP> <PUERTO>"
fi

URLS = ("1", "2", "3")

for i in {1..3}
do
	xterm -e ./bin/Cliente $1 $2 ${URLS[i]} "auto" &
done
