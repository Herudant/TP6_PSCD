//*****************************************************************
// File:    ImageDownloader.cpp
// Author:  PSCD-Unizar
// Date:    11/11/15
// Coms:    Programación de Sistemas Concurrentes y
//          Distribuidos Curso 2015-2016.
//          Implementación de una clase para descarga de imágenes de Internet
//*****************************************************************

#include "ImageDownloader.hpp"
#include <curl/curl.h>


ImageDownloader::ImageDownloader() {
    // Inicializamos la libreria de curl
    curl_global_init(CURL_GLOBAL_ALL);
}

ImageDownloader::~ImageDownloader() {
    // Limpiamos el espacio usado
    curl_global_cleanup();
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {

    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int ImageDownloader::downloadImage(char* URL, char* path) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        fp = fopen(path,"wb");
        curl_easy_setopt(curl, CURLOPT_URL, URL);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
        fclose(fp);

        return 0;
    } else
        return -1;
}
