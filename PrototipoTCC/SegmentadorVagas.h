#pragma once

#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sstream>
#include "Structs.h"

class SegmentadorVagas
{
public:
	SegmentadorVagas();
	float PicoHistograma(Mat histograma, int tamHist);
	float LimiarHistograma(Mat histograma, int tamHist, float pico, float porcentagem);
	void BinarizacomHist(Mat &m, float porcentagem, int limiarMinimo);
	void GiraImagem(Mat &m);
	void OperacoesMorfologicas(Mat &m);
	vector<Vaga> EncontraVagas(Mat m, bool girou);
	vector<Vaga> Segmentar(Mat img);
};

