//*****************************************************************
// File:    ImageDownloader.hpp
// Author:  PSCD-Unizar
// Date:    11/11/15
// Coms:    Programación de Sistemas Concurrentes y 
//          Distribuidos Curso 2015-2016.
//          Especificación de una librería para descarga de imágenes de Internet
//*****************************************************************


#ifndef IMAGE_DOWNLOADER_HPP
#define IMAGE_DOWNLOADER_HPP

class ImageDownloader {

public:

	// CONSTRUCTORES
	// Pre:
	// Post: Inicializa la librería de descarga de imágenes
	ImageDownloader();
	
	// DESTRUCTOR
	// Pre: 
	// Post: Libera el espacio utilizado para la descarga de imágenes. 
	//       Se llama de manera automática al cerrar el bloque en que se declaró el objeto	
	~ImageDownloader();
	
	// MÉTODOS

	// Pre:  "URL": URL correspondiente a una imagen
	//       "path": Ruta local donde almacenar la imagen
	//		 El método 'cleanup' no ha sido llamado
	// Post: Descarga una imagen de la URL y la almacena en el path local indicado
	// 		 Devuelve 0 en caso de éxito y -1 en caso de error
	int downloadImage(char* URL, char* path);
};
#endif