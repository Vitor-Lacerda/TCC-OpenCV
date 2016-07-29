#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

using namespace cv;
using namespace std;


float PicoHistograma(Mat histograma, int tamHist) {
	float pico = 0;
	for (int i = 0; i<tamHist; i++) {
		if (histograma.at<float>(i) > pico) {
			pico = histograma.at<float>(i);
		}
	}

	return pico;

}

float LimiarHistograma(Mat histograma, int tamHist, float pico, float porcentagem) {

	float limiar = pico * (porcentagem / 100);
	int cont = 0;
	for (int i = 0; i<tamHist; i++) {
		if (histograma.at<float>(i) < limiar) {
			cont++;
			if (cont >= 3) {
				limiar = i;
				break;
			}
		}
	}

	/*float limiar = 0;
	float valorTotal = sum(histograma)[0];
	float somaTotal = 0;

	for(int i = 0;i<tamHist;i++){
	if(somaTotal < valorTotal*(100-porcentagem)/100){
	somaTotal += histograma.at<float>(i);
	}
	else{
	limiar = i;
	break;
	}
	}*/


	return limiar;
}

void BinarizacomHist(Mat &m, float porcentagem, int limiarMinimo) {

	Mat histograma;
	int tamHist = 256;
	float range[] = { 0, 256 };
	const float* rangeHist = { range };

	threshold(m, m, limiarMinimo, 0, THRESH_TOZERO);

	calcHist(&m, 1, 0, Mat(), histograma, 1, &tamHist, &rangeHist, true, true);

	//Desenhar o histograma na tela
	/*
	int hist_w = 300; int hist_h = 160;
	int bin_w = cvRound( (double) hist_w/tamHist );
	Mat histogramaIMG( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
	for( int i = 1; i < tamHist; i++ )
	{
	line( histogramaIMG, Point( bin_w*(i-1), hist_h - cvRound(histograma.at<float>(i-1)*10) ) ,
		Point( bin_w*(i), hist_h - cvRound(histograma.at<float>(i)*10) ), Scalar( 255, 0, 0), 2, 8, 0  );

	}
	imshow("Histograma", histogramaIMG);
	*/

	//Encontra o maior valor do histograma
	float pico = PicoHistograma(histograma, tamHist);
	float limiar = LimiarHistograma(histograma, tamHist, pico, porcentagem);
	threshold(m, m, limiar, 255, CV_THRESH_BINARY);

}

Mat Diferenca(Mat a, Mat b, float porcentagem, int lm) {
	Mat D;
	
	absdiff(a, b, D);
	//imshow("D", D);
	//threshold(D, D, lm, 255, THRESH_BINARY);
	imshow("Absoluta", D);
	BinarizacomHist(D, porcentagem, lm);


	//imshow("DBin", D);
	return D;

}


int main(int argc, char** argv) {

	Mat imagem1, imagem2;
	Mat D;


	imagem1 = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	imagem2 = imread(argv[2], CV_LOAD_IMAGE_GRAYSCALE);

	GaussianBlur(imagem1, imagem1, Size(5, 5), 0);
	GaussianBlur(imagem2, imagem2, Size(5, 5), 0);

	D = Diferenca(imagem2, imagem1, 1, atoi(argv[2]));


	imshow("vazia", imagem1);
	imshow("ocupada", imagem2);
	imshow("Diferenca", D);

	waitKey(0);
	 
	return 0;
}