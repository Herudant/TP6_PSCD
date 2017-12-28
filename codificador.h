//============================================================================
// Name        :
// Author      : Eduardo Alonso Monge
// Description : practica 5 de pscd
//============================================================================

#ifndef COFIFICADOR_H_
#define COFIFICADOR_H_

#include <iostream>
#include <string>
#include <list>

using namespace std;

list<string> decodificar(string mensaje, const char separador){
  list<string> ret;
  string msg = "";
  for(char ch: mensaje){
    if(ch == separador){
      ret.push_back(msg);
			msg = "";
		}
    else
      msg+=ch;
  }
	ret.push_back(msg);
  return ret;
}

#endif /* VALLAPRINCIPAL_H_ */
