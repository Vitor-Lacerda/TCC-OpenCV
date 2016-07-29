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


class VerificadorVagas
{
public:
	VerificadorVagas();
	void Iniciar(vector<Mat> imagens, vector<Vaga> vagas);
	void Iniciar(VideoCapture video, vector<Vaga> vagas, Rect roiModelo, Rect roiVagas);
	void AtualizaIndicacoes(Mat & indicacoes, vector<Vaga>& vagas);
	void VerificaEstado(Mat quadro, Mat fundo, vector<Vaga>& vagas);
	bool ComparaComFundo(Mat fundo, Mat quadro, Vaga &vaga, int index);
	bool DeterminaOcupacao(Mat vaga, int index);
	int contaBrancos(Mat a);
};

