#include "Comparador.h"



Comparador::Comparador()
{
}

void Comparador::Iniciar(vector<Mat> imagens, vector<Vaga> vagas)
{
	Mat indicacoes;

	for (int i = 0; i < imagens.size(); i++) {
		if (i == 0) {
			indicacoes = Mat::zeros(imagens[0].rows, imagens[0].cols, CV_32FC3);
		}
		else {
			cout << "Momento  " << i << endl;

			ComparaImagens(indicacoes, imagens[i - 1], imagens[i], false, vagas);
			imshow("Antes", imagens[i - 1]);
			imshow("Depois", imagens[i]);
			imshow("Indicacoes", indicacoes);
			waitKey(0);
		}

	}

}




void Comparador::ComparaImagens(Mat &indicacoes, Mat a, Mat b, bool simples, vector<Vaga> &vagas) {


	Mat D = Diferenca(b, a, 0.1, 1);

	VerificaEstado(D, vagas, simples, a, b);



	indicacoes = Mat::zeros(indicacoes.size(), indicacoes.type());
	for (int i = 0; i < vagas.size(); i++) {
		Scalar cor = Scalar(255, 0, 0);
		if (vagas[i].ocupada) {
			cor = Scalar(0, 0, 255);
		}
		//circle(indicacoes, vagas[i].limites., 1, cor);
		rectangle(indicacoes, vagas[i].limites, cor, -1);
	}

}

Mat Comparador::Diferenca(Mat a, Mat b, float porcentagem, int lm) {
	Mat D;

	absdiff(a, b, D);
	//imshow("D", D);
	threshold(D, D, lm, 255, THRESH_BINARY);

	//BinarizacomHist(D, porcentagem, lm);
	Mat structuring = getStructuringElement(MORPH_RECT, cv::Size(3,5));

	//imshow("DBinaria", D);

	dilate(D, D, structuring, Point(0, 0), 5);
	erode(D, D, structuring, Point(0, 0), 5);

	//imshow("DBin", D);
	return D;

}

void Comparador::VerificaEstado(Mat D, vector<Vaga> &vagas, bool simples, Mat anterior, Mat atual) {

	for (int i = 0; i < vagas.size(); i++) {

		Rect areaTeste = vagas[i].limites;

		Mat Darea = D(areaTeste);
		int pixelsemDarea = Darea.cols*Darea.rows;

		//cout << "Brancos em D: " << contaBrancos(Darea) << "  Pixels em Darea:" << pixelsemDarea << endl;

		//string nome = "Retangulo vaga: ";
		//ostringstream oss;
		//oss << nome << i;

		rectangle(D, areaTeste, Scalar(255, 0, 0), 1, 8, 0);
		imshow("Diferenca", D);
		//imshow(oss.str(), Darea);

		if (contaBrancos(Darea) >= pixelsemDarea / 4) {
			if (simples) {
				vagas[i].ocupada = !vagas[i].ocupada;
			}
			else {
				vagas[i].ocupada = DeterminaOcupacao(areaTeste, i, anterior, atual);
			}

		}

	}

}

bool Comparador::DeterminaOcupacao(Rect retangulo, int indexVaga, Mat anterior, Mat atual) {

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
int Comparador::contaBrancos(Mat a) {
	int zeros = 0;
	vector<Mat> avector;

	split(a, avector);
	for (int i = 0; i<avector.size(); i++) {
		zeros += countNonZero(avector[i]);
	}
	merge(avector, a);


	return zeros;
}

