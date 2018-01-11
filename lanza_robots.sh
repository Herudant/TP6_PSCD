#! /bin/bash
if [ "$#" -ne 2 ]; then
    echo "Se esperaba ./lanza_robots <IP> <PUERTO>"
    exit 1
fi

URLS=("https://static.photocdn.pt/images/articles/2017/04/28/iStock-516651882.jpg" "https://static.pexels.com/photos/132037/pexels-photo-132037.jpeg" "https://i.ytimg.com/vi/c7oV1T2j5mc/maxresdefault.jpg")

for i in {1..3}
do
	xterm -e ./bin/Cliente $1 $2 ${URLS[i]} "auto" &
done
