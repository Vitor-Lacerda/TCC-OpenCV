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

int contaBrancos(Mat a) {
	int zeros = 0;
	vector<Mat> avector;

	split(a, avector);
	for (int i = 0; i<avector.size(); i++) {
		zeros += countNonZero(avector[i]);
	}
	merge(avector, a);


	return zeros;
}


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


/*
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
*/
Vaga novaVaga(Point centro = Point(0,0)) {
	Vaga novaVaga;
	novaVaga.centro = centro;
	novaVaga.ocupada = false;

	return novaVaga;
}
/*
Mat CalculaHistograma(Mat imagem) {
	Mat histograma;
	int tamHist[] = { 256 };
	float range[] = { 0, 255 };
	const float* rangeHist = { range };

	calcHist(&imagem, 1, 0, Mat(), histograma, 1, tamHist, &rangeHist, true, false);

	return histograma;

}
*/

/*
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
	
	
	
	//int tamHist[] = { 256 };
	//float range[] = { 0, 255 };
	//const float* rangeHist = { range };
	//int channels[] = { 0, 1 };

	//calcHist(&imagemHSV, 1, channels, Mat(), histHSV, 1, tamHist, &rangeHist, true, false);
	

	return histHSV;
	
}
*/

/*
bool ConfereOcupacao(Mat imagem, Rect retangulo, int indexVaga) {
	Mat regiao = Mat::zeros(imagem.rows, imagem.cols, imagem.type());
	Mat saidaCanny;

	float limiar = 400;

	regiao = imagem(retangulo);


	string nome = "Retangulo vaga: ";
	ostringstream oss;
	oss << nome << indexVaga;


	imshow(oss.str(), regiao);

	oss << " Canny";

	Canny(regiao, saidaCanny, 75, 150);

	imshow(oss.str(), saidaCanny);

	cout << "Brancos na imagem da vaga "<< indexVaga << ": " << contaBrancos(saidaCanny) << endl;

	return contaBrancos(saidaCanny) >= limiar;
}
*/

bool DeterminaOcupacao(Rect retangulo, int indexVaga, Mat anterior, Mat atual) {

	Mat regiaoA = Mat::zeros(atual.rows, atual.cols, atual.type());
	Mat regiaoB;
	regiaoA.copyTo(regiaoB);

	Mat cannyA, cannyB;

	//float limiar = 200;

	regiaoA = anterior(retangulo);
	regiaoB = atual(retangulo);

	Canny(regiaoA, cannyA, 75, 150);
	Canny(regiaoB, cannyB, 75, 150);

	

	cout << "Brancos antes na vaga " << indexVaga << ": " << contaBrancos(cannyA) << "   Brancos depois: " << contaBrancos(cannyB) << endl;

	int dif = contaBrancos(cannyB) - contaBrancos(cannyA);

	return contaBrancos(cannyB) > 300;
	

}

/*
void VerificaEstado(Mat D, vector<Vaga> &vagas, bool simples,Mat anterior, Mat atual) {

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

					//Rect areaTeste = minAreaRect(contornos[i]).boundingRect();
					Rect areaTeste = Rect(centro.x-15, centro.y-25, 50,75);

					vagas[j].ocupada = DeterminaOcupacao(areaTeste, j, anterior, atual);


				}
			}
		}
	}
}
*/

void VerificaEstado(Mat D, vector<Vaga> &vagas, bool simples, Mat anterior, Mat atual) {

	for (int i = 0; i < vagas.size(); i++) {

		Point2f centro = vagas[i].centro;
		Rect areaTeste = Rect(centro.x - 25, centro.y - 40, 50, 80);
		
		Mat Darea = D(areaTeste);
		int pixelsemDarea = Darea.cols*Darea.rows;

		//cout << "Brancos em D: " << contaBrancos(Darea) << "  Pixels em Darea:" << pixelsemDarea << endl;

		string nome = "Retangulo vaga: ";
		ostringstream oss;
		oss << nome << i;

		rectangle(D, areaTeste, Scalar(255, 0, 0), 1, 8, 0);
		imshow("Diferenca", D);
		//imshow(oss.str(), Darea);

		if (contaBrancos(Darea) >= pixelsemDarea/4) {
			if (simples) {
				vagas[i].ocupada = !vagas[i].ocupada;
			}
			else {
				vagas[i].ocupada = DeterminaOcupacao(areaTeste, i, anterior, atual);
			}

		}

	}

}


void ComparaImagens(Mat &indicacoes, Mat a, Mat b, bool simples, vector<Vaga> &vagas) {
	
	
	Mat D = Diferenca(b, a, 0.1, 1);
	
	VerificaEstado(D, vagas,simples, a, b);

	

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
	Mat indicacoes, D;
	vector<Mat> imagens;
	vector<Vaga> vagas;

	string endereco = "../ImagensTeste/Momento";


	for (int i = 0; i < 11; i++) {
		ostringstream oss;
		oss << endereco << i <<".png";
		Mat img = imread(oss.str(), CV_32FC3);
		if (img.empty()) {
			cout << " Nao carregou   "<< oss.str() << endl;
			break;
		}
		imagens.push_back(img);
		if (i == 0) {
			indicacoes = Mat::zeros(img.rows, img.cols, CV_32FC3);
			for (int c = 0; c < 11; c++) {
				vagas.push_back(novaVaga(Point(30 + 50 * c + 2*c , 330)));
				vagas.push_back(novaVaga(Point(30 + 50 * c + 2*c, 50)));

				//circle(img, Point(30 + 50 * c + 2 * c, 330), 1, Scalar(255, 0, 0));
				//circle(img, Point(30 + 50 * c + 2 * c, 50), 1, Scalar(255, 0, 0));

			}
			/*
			for (int j = 0; j < vagas.size(); j++) {
				Rect areaTeste = Rect(vagas[j].centro.x - 25, vagas[j].centro.y - 40, 50, 100);
				rectangle(img, areaTeste, Scalar(255, 0, 0), 1, 8, 0);
			}
			imshow("Rets", img);
			*/
		}
		else {

			cout << "Momento  " << i << endl;

			ComparaImagens(indicacoes, imagens[i - 1], imagens[i], false, vagas);
			imshow("Antes", imagens[i - 1]);
			imshow("Depois", imagens[i]);
			imshow("Indicacoes",indicacoes);
			waitKey(0);

		}
	}



	waitKey(0);

	return 0;
}