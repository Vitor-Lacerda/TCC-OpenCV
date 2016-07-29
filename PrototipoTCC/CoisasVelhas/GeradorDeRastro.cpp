#include <opencv\cv.h>
#include <opencv\highgui.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

using namespace cv;
using namespace std;

struct Rastro {
	Point inicio;
	Point fim;
	Point fimAnterior;
	int repeticoes;
	Scalar cor;

};


/*Uso pra gerar um fundo inicial*/
void MediaVariancia(int quadros, char* nomevid, Mat &mediaS, Mat &varianciaS) {

	Mat media;
	Mat variancia;
	Mat frame;
	VideoCapture vid(nomevid);

	vid.read(frame);
	media = Mat::zeros(frame.rows, frame.cols, frame.type());
	for (int i = 0; i<quadros; i++) {
		add(frame / quadros, media, media);
		vid.read(frame);

	}

	variancia = Mat::zeros(media.rows, media.cols, media.type());

	media.copyTo(mediaS);
	variancia.copyTo(varianciaS);

}

Mat Diferenca(Mat a, Mat b, float porcentagem, int lm) {
	Mat D;
	
	absdiff(a, b, D);
	threshold(D, D, lm, 255, THRESH_BINARY);
	//imshow("DBin", D);
	return D;

}

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

/*Se for diferente do quadro anterior, mostra o que tinha no fundo antes,
se for igual, mostra o que tem no quadro atual */
Mat AtualizaFundo(Mat ant, Mat mascara, Mat quadro) {

	Mat fundo;
	Mat temp;
	Mat media;

	float prop = 0.5;

	//Temp e a imagem que tem o conteudo do fundo anterior na regiao marcada pela mascara
	bitwise_and(ant, mascara, temp);
	//Inverte a mascara
	bitwise_not(mascara, mascara);
	//Faz uma "media" dos valores do fundo anterior com os do quadro atual. Mudar prop muda o peso de cada um
	add((1 - prop)*ant, (prop)*quadro, media);
	//Apaga de media os valores da regiao marcada pela mascara
	bitwise_and(media, mascara, media);
	//Coloca temp na regiao apagada e coloca tudo no fundo novo.
	add(media, temp, fundo);
	
	imshow("Media", media);
	imshow("Mascara", mascara);
	imshow("Temp", temp);
	
	return fundo;
}

float DistanciaEuclidiana(Point2f a, Point2f b) {

	Point2f p = b-a;
	return sqrt(p.x*p.x + p.y*p.y);

}


Rastro NovoRastro(Point inicio, Point fim, int seed) {
	Rastro novoRastro;
	RNG rng(seed);
	novoRastro.inicio = inicio;
	novoRastro.fim = fim;
	novoRastro.fimAnterior = fim;
	novoRastro.repeticoes = 0;
	novoRastro.cor = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	return novoRastro;


}

void DesenhaRastro(Mat area, Mat &rastro, vector<Rastro> &rastros) {

	vector<vector<Point>> contornos;
	vector<Vec4i> hierarquia;
	vector<Mat> areaSplit;
	int tamMinimo = 1000;

	bool crianovo = true;

	bitwise_not(area, area);
	//Pra mexer com um canal só
	split(area, areaSplit);
	//Encontra os contornos
	findContours(areaSplit[0], contornos, hierarquia, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);


	//Se o contorno for grande o suficiente (?) acha o centro e armazena
	Point2f centro(0,0);
	
	for (int i = 0; i<contornos.size(); i++) {
		if (contourArea(contornos[i]) >= tamMinimo) {
			float raio;
			minEnclosingCircle(contornos[i], centro, raio);
			//Se a coloeção de rastros estiver vazia cria um novo
			if (rastros.size() == 0) {
				//cout << "Criou um"<<novoRastro.inicio<<endl;
				rastros.push_back(NovoRastro(centro,centro,rastros.size()));
			}
			//Caso contrario confiro algumas coisas
			else {
				crianovo = true;
				//Se o centro atual estiver perto o suficiente do ponto final de algum rastro ele é o novo final
				for (int j = 0; j < rastros.size(); j++) {
					if (abs(DistanciaEuclidiana(rastros[j].fim, centro)) <= 50) {
						
						rastros[j].fimAnterior = rastros[j].fim;
						rastros[j].fim = centro;
						
						crianovo = false;
						//Desenha na imagem
						circle(rastro, centro, 5, rastros[j].cor, -1);
					}
					
				}
				//Se nao, inicia um rastro novo
				if (crianovo) {
					//cout << "Criou um" << novoRastro.inicio << endl;
					rastros.push_back(NovoRastro(centro,centro, rastros.size()));
				}
			}		
		}
	}

}

bool ConferePosicao(Rastro rastro, Point2f *roi){
	vector<Point2f> pontos;

	for (int i = 0; i < 4; i++){
		pontos.push_back(roi[i]);
	}
	//Começou fora terminou dentro
	if (pointPolygonTest(pontos, rastro.inicio, false) < 0 && pointPolygonTest(pontos, rastro.fim, false) >= 0) {
		cout << "Vagas livres -1\n";
		return true;
	}
	//Começou dentro terminou fora
	if (pointPolygonTest(pontos, rastro.inicio, false) >=0 && pointPolygonTest(pontos, rastro.fim, false)<0) {
		cout << "Vagas livres +1\n";
		return true;
	}

	return true;
}

void AnalisaRastros(vector<Rastro> &rastros, Point2f *roi) {
	for (int j = 0; j < rastros.size(); j++) {
		if(abs(DistanciaEuclidiana(rastros[j].fim, rastros[j].fimAnterior)) < 0.1f){
			rastros[j].repeticoes++;
			//cout << "Repeticoes "<< j << " = " << rastros[j].repeticoes << endl;
		}
		else {
			rastros[j].fimAnterior = rastros[j].fim;
		}
		if(rastros[j].repeticoes >= 3){
			if (ConferePosicao(rastros[j], roi)) {
				//cout << "Apagando rastro " << j << endl;
				rastros.erase(rastros.begin() + j);
			}
		}
	}
}

int main(int argc, char** argv) {

	


	Mat media, variancia;
	Mat fundo, quadroAnterior;
	Mat rastro;
	VideoCapture vid(argv[1]);
	vector<Rastro> rastros;
	RotatedRect conjuntoDasVagas = RotatedRect(Point2f(550, 150), Size2f(200,70), 70);
	int cont = 0;

	MediaVariancia(1, argv[1], media, variancia);
	media.copyTo(rastro);

	Point2f vertices[4];
	conjuntoDasVagas.points(vertices);

	for (int i = 0; i < 4; i++)
		line(rastro, vertices[i], vertices[(i + 1) % 4], Scalar(0, 255, 0));





	while (vid.isOpened()) {
		Mat quadro;
		if (!vid.read(quadro)) {
			break;
		}
		else {
			Mat D;
			//rectangle(quadro, Point2f(250, 140), Point2f(400, 500), Scalar(0, 0, 0), -1);
			if (!quadroAnterior.empty()) {
				D = Diferenca(quadro, quadroAnterior, 1, atoi(argv[2]));
				//Se os quadros não forem exatamente iguais atualiza o rastro
				if (contaZeros(D)>0) {
					Mat estrut;
					estrut = Mat::ones(6, 6, CV_8UC1);
					dilate(D, D, estrut, Point(-1, -1), 5);
					DesenhaRastro(D, rastro, rastros);
					cont++;
					if (cont > 15) {
						AnalisaRastros(rastros, vertices);
						cont = 0;
					}

				}
				//imshow("Dilatada", D);
				imshow("rastro", rastro);
				imshow("Video", quadro);
				//imshow("Anterior", quadroAnterior);

				waitKey(1);
			}

			quadro.copyTo(quadroAnterior);
		}
	}

	return 0;
}