#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sstream>
#include "Structs.h"
#include "SegmentadorVagas.h"
#include "VerificadorVagas.h"

using namespace cv;
using namespace std;

int contaImagens = 121;

struct MouseParams {
	Mat img;
	int pontosMarcados;
	Point2f ponto1, ponto2, ponto3, ponto4;
	vector<Rect> ROIs;
	vector<Vaga> vagas;
	Rect vagaModelo;
	Rect regiaoBusca;
};


Rect fazerRetangulo(Point2f a, Point2f b, Point2f c, Point2f d) {

	Point2f pontos[4] = { a,b,c,d };
	int menorX, maiorX, menorY, maiorY, largura, altura;
	menorX = INT_MAX;
	maiorX = 0;
	menorY = INT_MAX;
	maiorY = 0;

	for (int c = 0; c < 4; c++) {
		if (pontos[c].x < menorX) {
			menorX = pontos[c].x;
		}
		if (pontos[c].x > maiorX) {
			maiorX = pontos[c].x;
		}
		if (pontos[c].y < menorY) {
			menorY = pontos[c].y;
		}
		if (pontos[c].y > maiorY) {
			maiorY = pontos[c].y;
		}
	}

	largura = maiorX - menorX;
	altura = maiorY - menorY;
	

	return Rect(menorX, menorY, largura, altura);
}

void vagaIndividual(int event, int x, int y, int, void *param) {
	//cout << "Individual" << endl;

	MouseParams* p = (MouseParams*)param;

	if (event == EVENT_LBUTTONUP) {
		//cout << x << " " << y << endl;
		if (p->pontosMarcados == 0) {
			p->ponto1 = Point2f(x, y);
			p->pontosMarcados++;
		}
		else if (p->pontosMarcados == 1) {
			p->ponto2 = Point2f(x, y);
			p->pontosMarcados++;
		}
		else if (p->pontosMarcados == 2) {
			p->ponto3 = Point2f(x, y);
			p->pontosMarcados++;
		}
		else if (p->pontosMarcados == 3) {
			p->ponto4 = Point2f(x, y);
			p->pontosMarcados = 0;
			Rect novoRect = fazerRetangulo(p->ponto1, p->ponto2, p->ponto3, p->ponto4);
			rectangle(p->img, novoRect, Scalar(255, 255, 0), 1);
			cout << "Fez vaga" << endl;
			p->vagas.push_back(Vaga(novoRect, false));
		}

	}

	
	param = p;

}



void regiaoCallback(int event, int x, int y, int, void* param) {

	//cout << "MainCallBack" << endl;

	MouseParams* p = (MouseParams*)param;

	if (event == EVENT_LBUTTONUP) {
		//cout << x << " " << y << endl;
		if (p->pontosMarcados == 0) {
			p->ponto1 = Point2f(x, y);
			
			p->pontosMarcados++;
		}
		else if (p->pontosMarcados == 1) {
			p->ponto2 = Point2f(x, y);
			p->pontosMarcados++;
		}
		else if (p->pontosMarcados == 2) {
			p->ponto3 = Point2f(x, y);
			p->pontosMarcados++;
		}
		else if (p->pontosMarcados == 3) {
			p->ponto4 = Point2f(x, y);
			p->pontosMarcados = 0;
			Rect novoRect = fazerRetangulo(p->ponto1, p->ponto2, p->ponto3, p->ponto4);
			p->ROIs.push_back(novoRect);

			p->regiaoBusca = novoRect;
			rectangle(p->img, novoRect, Scalar(255, 255, 0), 1);

			/*
			SegmentadorVagas sv;
			vector<Vaga> vect = sv.Segmentar(p->img(novoRect));
			for each (Vaga v in vect)
			{
				int x = novoRect.x + v.limites.x;
				int y = novoRect.y + v.limites.y;
				Rect r = Rect(x, y, v.limites.width, v.limites.height);

				p->vagas.push_back(Vaga(r, false));
				rectangle(p->img, r, Scalar(255, 255, 0), 1);
				
			}
			*/


		}

	}

	/*
	if (p->vagas.size() > 0) {
		for each (Vaga v in p->vagas)
		{
			rectangle(p->img, v.limites, Scalar(0, 255, 0), 1);
		}
	}
	*/
	
	
	param = p;


}

void modeloCallback(int event, int x, int y, int, void* param) {
	MouseParams* p = (MouseParams*)param;

	if (event == EVENT_LBUTTONUP) {
		//cout << x << " " << y << endl;
		if (p->pontosMarcados == 0) {
			p->ponto1 = Point2f(x, y);
			p->pontosMarcados++;
		}
		else if (p->pontosMarcados == 1) {
			p->ponto2 = Point2f(x, y);
			p->pontosMarcados++;
		}
		else if (p->pontosMarcados == 2) {
			p->ponto3 = Point2f(x, y);
			p->pontosMarcados++;
		}
		else if (p->pontosMarcados == 3) {
			p->ponto4 = Point2f(x, y);
			p->pontosMarcados = 0;
			Rect novoRect = fazerRetangulo(p->ponto1, p->ponto2, p->ponto3, p->ponto4);
			rectangle(p->img, novoRect, Scalar(255, 255, 0), 1);
			cout << "Fez vaga" << endl;
			p->vagaModelo = novoRect;
		}

	}
}

void Ajuda() {
	cout << "Comandos:" << endl;
	cout << "r: Determinar uma regiao de interesse aonde o programa automaticamente tenta encontrar as vagas." << endl;
	cout << "t: Determinar manualmente uma regiao que deve ser considerada como uma vaga para o programa." << endl;
	cout << "q: Sai do programa." << endl;

}

void ExtraiImagens(Mat img, Rect regiaoVagas) {

	int largura = regiaoVagas.width/30;
	int x = regiaoVagas.x;
	int y = regiaoVagas.y;

	while (x < (regiaoVagas.x + regiaoVagas.width) - largura) {

		Rect secao = Rect(x, y, largura, regiaoVagas.height);
		Mat secaoIMG = img(secao);

		stringstream oss;
		oss << "../ImagensTeste/Extraidas/Secao" << contaImagens << ".png";
		imwrite(oss.str(), secaoIMG);



		x += largura;
		contaImagens++;
	}
}

int main(int argc, char** argv[]) {
	
	MouseParams mps;
	//vector<Mat> imagens;

	Mat imagemToda;
	

	//string endereco = "../ImagensTeste/Animacao/Animacao";
	//string endereco = "../ImagensTeste/Vazio.mp4";
	/*
	string endereco = "../ImagensTeste/MeioCheioCortado.mp4";

	VideoCapture video;
	video = VideoCapture(endereco);
	video.read(imagemToda);
	cvtColor(imagemToda, imagemToda, CV_BGR2GRAY);
	imshow("Imagem Inicial", imagemToda);
	mps.img = imagemToda;
	mps.pontosMarcados = 0;

	*/

	string endereco = "../ImagensTeste/Vazio3.png";
	imagemToda = imread(endereco);
	cvtColor(imagemToda, imagemToda, CV_BGR2GRAY);
	imshow("Imagem Inicial", imagemToda);
	mps.img = imagemToda;
	mps.pontosMarcados = 0;

	/*
	for (int i = 0; i < 84; i++) {
		ostringstream oss;
		oss << endereco << i << ".png";
		Mat img = imread(oss.str(), 0);
		imagens.push_back(img);
	}

	imagemToda = imagens[0];

	mps.pontosMarcados = 0;
	imshow("Imagem Inicial", imagemToda);

	mps.img = imagemToda;
	*/



	cout << "Bem vindo ao programa. Para começar aperte a tecla referente a função desejada. Pressione h para ajuda." << endl;
	
	while (true) {
		int key;
		key = waitKey(1);
		if (key == 'q') {
			break;
		}
		if (key == 'r') {
			cout << "Clique nos 4 pontos que determinam a regiao de interesse" << endl;
			setMouseCallback("Imagem Inicial", regiaoCallback, (void*)&mps);
		}
		if (key == 't') {
			cout << "Clique nos 4 pontos que determinam a vaga" << endl;
			setMouseCallback("Imagem Inicial", vagaIndividual, (void*)&mps);
		}
		if (key == 'm'){
			cout << "Clique nos 4 pontos para determinar a regiao modelo" << endl;
			setMouseCallback("Imagem Inicial", modeloCallback, (void*)&mps);
		}
		if (key == 'i') {
			cout << "Começou a analisar" << endl;
			VerificadorVagas cmp;
			mps.vagaModelo = mps.vagas[0].limites;
			//cmp.Iniciar(video, mps.vagas, mps.vagaModelo, mps.regiaoBusca);
		}

		if (key == 's') {
			cout << "Extraindo imagens" << endl;
			ExtraiImagens(imagemToda, mps.regiaoBusca);
		}

		if (key == 'h') {
			Ajuda();
		}


		imshow("Imagem Inicial",mps.img);

	}


	//waitKey(0);


	return 0;
}