//*****************************************************************
// File:    demo.cpp
// Author:  PSCD-Unizar
// Date:    11/11/15
// Coms:    Programación de Sistemas Concurrentes y
//          Distribuidos Curso 2015-2016.
//          Demostración de uso de la librería "ImageDownloader" para
//          la descarga de imágenes de Internet y de la librería "CImg"
//          para la visualización de imágenes
//          Compilar mediante "make"
//*****************************************************************

#include <iostream>
#include <string>
#include "ImageDownloader.hpp"
#include "CImg.h"

using namespace std;
using namespace cimg_library;

int main() {
    // Tamaños de ventana para las vallas publicitarias
    const int VALLA_WIDTH = 800;
    const int VALLA_HEIGHT = 800;

    // Creamos el objeto para descargar imágenes
    ImageDownloader downloader;
    // Descargamos una imagen de Internet
    //que se almacenará como "IC343.jpg"
    char URL[500] = "http://assets2.motherboard.tv/content-images/contentimage/no-slug/7703bcea71c8096ea7c18addc28e8a15.jpg";
    char path[100] = "IC343.jpg";
    downloader.downloadImage(URL, path);

    // Creamos una valla publicitaria con una imagen
    cimg_library::CImg<unsigned char> img_principal(path);
    cimg_library::CImgDisplay valla(img_principal.resize(VALLA_WIDTH,VALLA_HEIGHT), "Valla 1");
    valla.resize(VALLA_WIDTH,VALLA_HEIGHT);
    valla.move(0,0); // Esquina superior izquierda

    // Mostrar imagen durante 5 segundos
	valla.wait(5000);
    return 0;
}
