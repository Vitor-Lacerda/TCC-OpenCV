#pragma once

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sstream>
#include "Structs.h"

using namespace cv;
using namespace std;

class Comparador

{
public:
	Comparador();

	void Iniciar(vector<Mat> imagens, vector<Vaga> vagas);

	void ComparaImagens(Mat & indicacoes, Mat a, Mat b, bool simples, vector<Vaga>& vagas);

	Mat Diferenca(Mat a, Mat b, float porcentagem, int lm);

	void VerificaEstado(Mat D, vector<Vaga>& vagas, bool simples, Mat anterior, Mat atual);

	bool DeterminaOcupacao(Rect retangulo, int indexVaga, Mat anterior, Mat atual);

	int contaBrancos(Mat a);
};

