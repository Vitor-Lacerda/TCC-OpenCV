#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

using namespace cv;
using namespace std;

struct Vaga {
	Point centro;
	bool ocupada;
};

/*
float PicoHistograma(Mat histograma, int tamHist) {
	float pico = 0;
	for (int i = 0; i<tamHist; i++) {
		if (histograma.at<float>(i) > pico) {
			pico = histograma.at<float>(i);
		}
	}

	return pico;

}
*/
/*
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

	float limiar = 0;
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
	}


	return limiar;
}
*/
/*
void BinarizacomHist(Mat &m, float porcentagem, int limiarMinimo) {

	Mat histograma;
	int tamHist = 256;
	float range[] = { 0, 256 };
	const float* rangeHist = { range };

	threshold(m, m, limiarMinimo, 0, THRESH_TOZERO);

	calcHist(&m, 1, 0, Mat(), histograma, 1, &tamHist, &rangeHist, true, true);


	//Encontra o maior valor do histograma
	float pico = PicoHistograma(histograma, tamHist);
	float limiar = LimiarHistograma(histograma, tamHist, pico, porcentagem);
	threshold(m, m, limiar, 255, CV_THRESH_BINARY);

}
*/

Mat Diferenca(Mat a, Mat b, float porcentagem, int lm) {
	Mat D;
	
	absdiff(a, b, D);
	//imshow("D", D);
	threshold(D, D, lm, 255, THRESH_BINARY);

	//BinarizacomHist(D, porcentagem, lm);


	//imshow("DBin", D);
	return D;

}
/*
int contaZeros(Mat a) {
	int zeros = 0;
	vector<Mat> avector;

	split(a, avector);
	for (int i = 0; i<avector.size(); i++) {
		zeros += countNonZero(avector[i]);
	}
	merge(avector, a);


	return zeros;
}
*/

float DistanciaEuclidiana(Point2f a, Point2f b) {

	Point2f p = b - a;
	return sqrt(p.x*p.x + p.y*p.y);

}


Mat Branco3Canais(Mat imagem) {
	Mat branca;

	vector<Mat> avector;
	split(imagem, avector);
	avector[0].copyTo(branca);

	imshow("1", avector[0]);
	imshow("2", avector[1]);
	imshow("3", avector[2]);

	//Cria uma imagem que e branca nos pontos onde algum dos canais rgb é branco.
	for (int i = 0; i<avector.size(); i++) {
		threshold(avector[i], avector[i], 0, 255, THRESH_BINARY);
		bitwise_or(branca, avector[i], branca);
	}


	imshow("branca", branca);

	return branca;
}
/*
Mat RegioesDiferenca(Mat fundo, Mat diferenca, float contornoMinimo) {
	Mat rD;

	vector<Mat> avector;
	split(diferenca, avector);


	Mat mancha = Branco3Canais(diferenca);

	imshow("Mancha", mancha);

	//Cria a imagem que mostra a região do fundo atual onde a tem a mancha branca
	split(fundo, avector);
	for (int i = 0; i<avector.size(); i++) {
		bitwise_and(avector[i], mancha, avector[i]);
	}
	
	merge(avector, rD);
	return rD;
}
*/
/*
void AnalisaDiferencasFundo(Mat regioes, Point2f *roi) {

	vector<vector<Point>> contornos;
	vector<Vec4i> hierarquia;
	//vector<Mat> regioesSplit;
	int tamMinimo = 15;

	bitwise_not(regioes, regioes);
	//split(regioes, regioesSplit);
	//Encontra os contornos
	//imshow("Canal 1 Regioes", regioesSplit[0]);
	findContours(regioes, contornos, hierarquia, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	Point2f centro(0, 0);
	
	for (int i = 0; i < contornos.size(); i++) {
		float raio;
		minEnclosingCircle(contornos[i], centro, raio);
		vector<Point2f> pontos;

		for (int i = 0; i < 4; i++) {
			pontos.push_back(roi[i]);
		}
		//Começou fora terminou dentro
		if (pointPolygonTest(pontos, centro, false) >= 0) {
			cout << raio << endl;
			if (raio >= tamMinimo) {
				cout << "Vagas disponíveis -1\n";
			}
		}
		
	}
}
*/



Mat DesenhaHistograma(Mat histograma, int tamHist) {
	int hist_w = 300; int hist_h = 160;
	int bin_w = cvRound((double)hist_w / tamHist);
	Mat histogramaIMG(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	for (int i = 1; i < tamHist; i++)
	{
		line(histogramaIMG, Point(bin_w*(i - 1), hist_h - cvRound(histograma.at<float>(i - 1) * 10)),
			Point(bin_w*(i), hist_h - cvRound(histograma.at<float>(i) * 10)), Scalar(255, 0, 0), 2, 8, 0);

	}

	return histogramaIMG;

}

Vaga novaVaga(Point centro = Point(0,0)) {
	Vaga novaVaga;
	novaVaga.centro = centro;
	novaVaga.ocupada = false;

	return novaVaga;
}



void VerificaContornos(Mat D, vector<Vaga> &vagas, Mat imagem) {

	vector<vector<Point>> contornos;
	vector<Vec4i> hierarquia;


	findContours(D, contornos, hierarquia, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	for (int i = 0; i < contornos.size(); i++) {
		for (int j = 0; j < vagas.size(); j++) {
			if (pointPolygonTest(contornos[i], vagas[j].centro, false)>=0) {
				vagas[j].ocupada = true;
			}
		}
	}
}


int main(int argc, char** argv) {
	Mat momento2, momento1, indicacoes, D;
	vector<Vaga> vagas;




	momento1 = imread("../Vazia.png", CV_LOAD_IMAGE_GRAYSCALE);
	momento2 = imread("../2Carros.png", CV_LOAD_IMAGE_GRAYSCALE);

	if (momento2.empty() || momento2.empty())
	{
		cout << "Could not open or find the image" << std::endl;
		waitKey(0);
		return -1;
	}

	indicacoes = Mat::zeros(momento2.rows, momento2.cols, CV_32FC3);
	
	for (int i = 0; i < 11; i++) {
		vagas.push_back(novaVaga(Point(30 + 50*i, 330)));
		vagas.push_back(novaVaga(Point(30 + 50 * i, 50)));
	}

	


	imshow("Momento1", momento1);
	imshow("Momento2", momento2);
	

	D = Diferenca(momento2, momento1, 0.1, 1);

	Mat structuring = getStructuringElement(MORPH_RECT, cv::Size(3, 5));

	//imshow("DBinaria", D);

	dilate(D, D, structuring, Point(0, 0), 5);
	erode(D, D, structuring, Point(0, 0), 5);

	imshow("DClosing",D);

	VerificaContornos(D, vagas, momento2);


	for (int i = 0; i < vagas.size(); i++) {
		Scalar cor = Scalar(255, 0, 0);
		if (vagas[i].ocupada) {
			cor = Scalar(0, 0, 255);
		}
		circle(indicacoes, vagas[i].centro, 5, cor,-1);
	}

	imshow("Indicacoes", indicacoes);

	waitKey(0);

	return 0;
}