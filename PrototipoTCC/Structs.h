#pragma once

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sstream>

using namespace cv;
using namespace std;


struct Vaga
{
	Rect limites;
	bool ocupada;
	bool olhando;

	Vaga(Rect _limites, bool _ocupada) {
		limites = _limites;
		ocupada = _ocupada;
		olhando = false;
	}

};


