#include "VerificadorVagas.h"



VerificadorVagas::VerificadorVagas()
{
}

void VerificadorVagas::Iniciar(vector<Mat> imagens, vector<Vaga> vagas)
{
	Mat indicacoes;
	for (int i = 0; i < imagens.size(); i++) {
		if (i == 0) {
			indicacoes = Mat::zeros(imagens[0].rows, imagens[0].cols, CV_32FC3);
			for (int j = 0; j < vagas.size(); j++) {
				DeterminaOcupacao(imagens[0](vagas[j].limites));
			}
		}
		else {
			cout << "Momento  " << i << endl;

			VerificaEstado(imagens[i], imagens[i-1], vagas);
			AtualizaIndicacoes(indicacoes, vagas);
			//imshow("Antes", imagens[i - 1]);
			imshow("Depois", imagens[i]);
			imshow("Indicacoes", indicacoes);
			waitKey(33);
		}

	}

}

void VerificadorVagas::AtualizaIndicacoes(Mat &indicacoes, vector<Vaga> &vagas)
{
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

void VerificadorVagas::VerificaEstado(Mat quadro, Mat fundo,  vector<Vaga> &vagas)
{

	for (int i = 0; i < vagas.size(); i++) {
		Mat imgVaga = quadro(vagas[i].limites);
		//vagas[i].ocupada = DeterminaOcupacao(imgVaga);
		vagas[i].ocupada = ComparaComFundo(fundo, quadro, vagas[i]);
	}

}

bool VerificadorVagas::ComparaComFundo(Mat fundo, Mat quadro, Vaga vaga) {
	
	Mat vagaQuadro = quadro(vaga.limites);
	Mat vagaFundo = fundo(vaga.limites);

	Mat D;
	absdiff(vagaQuadro, vagaFundo, D);
	threshold(D, D, 1, 255, THRESH_BINARY);

	//imshow("D", D);

	if (contaBrancos(D) >= 0) {
		return DeterminaOcupacao(vagaQuadro);
	}






}

bool VerificadorVagas::DeterminaOcupacao(Mat vaga) {

	

	Mat canny;
	Canny(vaga, canny, 75, 150);

	//cout << "Brancos antes na vaga " << indexVaga << ": " << contaBrancos(cannyA) << "   Brancos depois: " << contaBrancos(cannyB) << endl;


	return contaBrancos(canny) > 400;


}

int VerificadorVagas::contaBrancos(Mat a) {
	int zeros = 0;
	vector<Mat> avector;

	split(a, avector);
	for (int i = 0; i<avector.size(); i++) {
		zeros += countNonZero(avector[i]);
	}
	merge(avector, a);


	return zeros;
}
