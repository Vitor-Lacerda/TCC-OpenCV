#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sstream>

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
	Mat structuring = getStructuringElement(MORPH_RECT, cv::Size(3, 5));

	//imshow("DBinaria", D);

	dilate(D, D, structuring, Point(0, 0), 5);
	erode(D, D, structuring, Point(0, 0), 5);

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
	int hist_w = 300; int hist_h = 300;
	int bin_w = cvRound((double)hist_w / tamHist);
	Mat histogramaIMG(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));

	//normalize(histograma, histograma, 0, histogramaIMG.rows, NORM_MINMAX, -1, Mat());

	for (int i = 1; i < tamHist; i++)
	{
		line(histogramaIMG, Point(bin_w*(i - 1), hist_h ),
			Point(bin_w*(i), hist_h  - histograma.at<float>(i)), Scalar(255, 0, 0), 2, 8, 0);

	}

	

	return histogramaIMG;

}

Vaga novaVaga(Point centro = Point(0,0)) {
	Vaga novaVaga;
	novaVaga.centro = centro;
	novaVaga.ocupada = false;

	return novaVaga;
}

Mat CalculaHistograma(Mat imagem) {
	Mat histograma;
	int tamHist[] = { 256 };
	float range[] = { 0, 255 };
	const float* rangeHist = { range };

	calcHist(&imagem, 1, 0, Mat(), histograma, 1, tamHist, &rangeHist, true, false);

	return histograma;

}

MatND HistogramaHSVNormalizado(Mat imagem, string nome) {

	Mat imagemHSV;

	//cvtColor(imagem, imagemHSV, CV_GRAY2BGR);
	cvtColor(imagem, imagemHSV, CV_BGR2HSV);

	imshow(nome, imagemHSV);

	MatND histHSV;


	
	int h_bins = 50; int s_bins = 60;
	int histSize[] = { h_bins, s_bins };

	float h_ranges[] = { 0, 180 };
	float s_ranges[] = { 0, 256 };

	const float* ranges[] = { h_ranges, s_ranges };

	int channels[] = { 0, 1 };
	

	calcHist(&imagemHSV, 1, channels, Mat(), histHSV, 2, histSize, ranges, true, false);
	
	
	/***
	int tamHist[] = { 256 };
	float range[] = { 0, 255 };
	const float* rangeHist = { range };
	int channels[] = { 0, 1 };

	calcHist(&imagemHSV, 1, channels, Mat(), histHSV, 1, tamHist, &rangeHist, true, false);
	*/

	return histHSV;

}


void ConfereOcupacao(Mat imagem, Rect retangulo, int indexVaga, MatND histogramaControle) {
	Mat regiao = Mat::zeros(imagem.rows, imagem.cols, imagem.type());
	Mat histograma;
	vector<vector<Point2f>> contornos;
	vector<Vec4i> hierarquia;
	//Rect retangulo;
	Mat histogramaImg;

	double dHists1, dHists2;

	string nome = "Retangulo vaga: ";
	ostringstream oss;
	oss << nome << indexVaga;

	//retangulo = rect.boundingRect();

	regiao = imagem(retangulo);
	
	//histograma = CalculaHistograma(regiao);
	histograma = HistogramaHSVNormalizado(regiao, oss.str());
	//imshow(oss.str(), regiao);
	
	//dHists1 = compareHist(histograma, histogramaControle, CV_COMP_CHISQR);

	//normalize(histograma, histograma, 0, 1, NORM_MINMAX, -1, Mat());
	//normalize(histogramaControle, histogramaControle, 0, 1, NORM_MINMAX, -1, Mat());

	dHists2 = compareHist(histograma, histogramaControle, CV_COMP_INTERSECT);



	//histogramaImg = DesenhaHistograma(histograma, 256);
	//imshow(oss.str(), histogramaImg);
	oss << "  Imagem";
	//imshow(oss.str(), regiao);


	//cout << oss.str() << "CHI^2: " <<  dHists1 << endl;
	cout << oss.str() << "  COMP:   " << dHists2 << endl;

}


void VerificaContornos(Mat D, vector<Vaga> &vagas, Mat imagem, bool simples, MatND histogramaControle, Mat imagemBGR) {

	vector<vector<Point>> contornos;
	vector<Vec4i> hierarquia;

	findContours(D, contornos, hierarquia, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	for (int i = 0; i < contornos.size(); i++) {
		for (int j = 0; j < vagas.size(); j++) {
			if (pointPolygonTest(contornos[i], vagas[j].centro, false)>=0) {
				if (simples) {
					vagas[j].ocupada = !vagas[j].ocupada;
				}
				else {

					Point2f centro;
					float raio;
					minEnclosingCircle(contornos[i], centro, raio);

					Rect areaTeste = Rect(centro.x, centro.y, 30,30);

					ConfereOcupacao(imagemBGR, areaTeste, j, histogramaControle);
				}
			}
		}
	}
}


void ComparaImagens(Mat &indicacoes, Mat a, Mat b, bool simples, vector<Vaga> &vagas, MatND histogramaControle) {
	
	Mat agrey, bgrey;

	cvtColor(a, agrey, CV_BGR2GRAY);
	cvtColor(b, bgrey, CV_BGR2GRAY);

	Mat D = Diferenca(bgrey, agrey, 0.1, 1);
	imshow("Diferenca", D);
	VerificaContornos(D, vagas, bgrey, simples, histogramaControle, b);

	

	indicacoes = Mat::zeros(indicacoes.size(), indicacoes.type());
	for (int i = 0; i < vagas.size(); i++) {
		Scalar cor = Scalar(255, 0, 0);
		if (vagas[i].ocupada) {
			cor = Scalar(0, 0, 255);
		}
		circle(indicacoes, vagas[i].centro, 1, cor);
	}

}



int main(int argc, char** argv) {
	Mat momento2, momento1, momento0, indicacoes, D;
	MatND histogramaControle;
	vector<Vaga> vagas;

	

	momento0 = imread("../Vazia.png", 1);
	momento1 = imread("../2Carros.png", 1);
	momento2 = imread("../4Carros.png", 1);


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

	

	

	imshow("Momento0", momento0);
	imshow("Momento1", momento1);
	imshow("Momento2", momento2);

	Rect retangulo = Rect(30, 50, 30, 30);

	//histogramaControle = CalculaHistograma(momento0(retangulo));
	histogramaControle = HistogramaHSVNormalizado(momento0(retangulo), "RegiaoControle");
	

	//imshow("Controle", DesenhaHistograma(histogramaControle, 256));

	
	cout << mean(momento0(retangulo)) << endl;
	
	//imshow("RegiaoControle", momento0(retangulo));
	/*
	ComparaImagens(indicacoes, momento0, momento1, true, vagas, histogramaControle);

	imshow("IndicacoesMomento1", indicacoes);
	*/


	ComparaImagens(indicacoes, momento1, momento2, false, vagas, histogramaControle);
	

	imshow("IndicacoesMomento2", indicacoes);

	waitKey(0);

	return 0;
}