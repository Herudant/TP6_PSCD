#! /bin/bash
if [ "$#" -ne 2 ]; then
    echo "Se esperaba ./lanza_robots <IP> <PUERTO>"
    exit 1
fi

URLS=("1" "2" "3")

for i in {0..2}
do
  echo $1 $2 ${URLS[i]} "auto"
	xterm -e ./bin/Cliente $1 $2 ${URLS[i]} "auto" &
done
