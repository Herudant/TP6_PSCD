#! /bin/bash
if [ "$#" -ne 3 ]; then
    echo "Se esperaba ./lanza_cliente_manual <IP> <PUERTO> <URL>"
    exit 1
fi

./bin/Cliente $1 $2 $3  #http://www.edsaplan.com/files/media-image/portfolio/hilton-yuxi-fuxian-lake-2400.jpg
